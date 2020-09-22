#include "TcpTransport.h"

#include <iostream>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>

TcpTransport::TcpTransport(boost::shared_ptr<boost::asio::io_context> ioc, boost::shared_ptr<boost::asio::ip::tcp::socket> socket, time_t timeout, size_t block_size) :
    BaseTransport<boost::asio::ip::tcp::resolver::results_type, tcp::TOnEvent> (ioc, timeout, block_size),
    m_socket(socket),
    m_read_data(new char[block_size]),
    m_read_data_length(block_size),
    m_messages()
{

}

TcpTransport::~TcpTransport()
{
    if (!m_read_data)
    {
        delete [] m_read_data;
    }
}

void TcpTransport::status(int32_t status) { m_transport_status = status; }

void TcpTransport::connect(const boost::asio::ip::tcp::resolver::results_type &endpoints)
{
    m_endpoints = endpoints;

    connect();
}

void TcpTransport::connect()
{
    for (auto v: m_messages)
    {
        m_allocator.destroy(v);
        m_allocator.deallocate(v, 1);
    }
    m_messages.clear();

    m_transport_status = transport::EN_READY;

    boost::asio::async_connect(
                *m_socket, m_endpoints,
                boost::bind(&TcpTransport::handle_connect, shared_from_this(), boost::asio::placeholders::error)
                );
    m_transport_status = transport::EN_CONNECTING;
}

void TcpTransport::disconnect()
{
    boost::asio::post(m_strand, boost::bind(&TcpTransport::handle_close, shared_from_this()));
}

int32_t TcpTransport::status()
{
    return m_transport_status;
}

void TcpTransport::connection_made()
{
    boost::system::error_code set_option_err;
    boost::asio::ip::tcp::no_delay no_delay(true);

    m_socket->set_option(no_delay, set_option_err);
    m_transport_status = transport::EN_OK;
    m_local_endpoint = m_socket->local_endpoint();
    m_remote_endpoint = m_socket->remote_endpoint();
    // 接收数据
    do_read();
}

void TcpTransport::write(const std::string &data, boost::function<void(const std::string &)> handle_error)
{
    boost::asio::post(m_strand, [this, data, handle_error]()
    {
        bool trigger = !m_messages.empty();

        // 通讯状态OK且缓存队列未满，将消息加入缓存队列
        if (m_messages.size() < 10000 && transport::EN_OK == status())
        {
            auto s = m_allocator.allocate(1);
            m_allocator.construct(s, data);

            m_messages.push_back(s);

            // 如果push前队列为空，手动调用do_write消费队列数据
            if (!trigger)
            {
                do_write();
            }
        }
        else
        {
            if (handle_error)
            {
                handle_error(data);
            }
        }
    });
}

boost::asio::ip::tcp::endpoint TcpTransport::endpoint(int32_t type)
{
    return EN_LOCAL_ENDPOINT == type ? m_local_endpoint : m_remote_endpoint;
}

void TcpTransport::handle_connect(const boost::system::error_code &err)
{
    if (!err)
    {
        boost::system::error_code set_option_err;
        boost::asio::ip::tcp::no_delay no_delay(true);

        m_socket->set_option(no_delay, set_option_err);
        m_local_endpoint = m_socket->local_endpoint();
        m_remote_endpoint = m_socket->remote_endpoint();
        if (!set_option_err)
        {
            m_transport_status = transport::EN_OK;
            if (m_on_events.has<tcp::on_connected>("on_connected"))
            {
                m_on_events.get<tcp::on_connected>("on_connected")();
            }
            // 接收数据
            do_read();
        }
    }
    else
    {
        m_transport_status = transport::EN_CLOSE;
        std::cout << "handle_connect error, messsage: " << err.message() << std::endl;
        if (m_on_events.has<tcp::on_connection_failed>("on_connection_failed"))
        {
            m_on_events.get<tcp::on_connection_failed>("on_connection_failed")(shared_from_this(), err);
        }
    }
}

void TcpTransport::handle_read(const boost::system::error_code &err, size_t length)
{
    if(!err)
    {
        // std::cout << "handle read: " << length << std::endl;

        // 处理接收的数据
        if (m_on_read)
        {
            m_on_read(std::string(m_read_data, length));
        }

        // 接收数据
        do_read();
    }
    else
    {
        std::cout << "handle_read error, message: " << err.message() << std::endl;
        handle_close();
        if (m_on_events.has<tcp::on_connection_lost>("on_connection_lost"))
        {
            m_on_events.get<tcp::on_connection_lost>("on_connection_lost")(shared_from_this(), err);
        }
    }
}

void TcpTransport::handle_write(const boost::system::error_code &err, size_t length)
{
    if (!err)
    {
        // std::cout << "actually sends " << length << " bytes" << std::endl;

        if (!m_messages.empty())
        {
            // std::cout << "buffer size: " << m_messages.front()->size() <<  ", actually sends " << length << " bytes" << std::endl;
            if (m_messages.front()->size() != length)
            {
                std::cout << "== error: " << m_messages.front()->size() <<  " != " << length << std::endl;
                return ;
            }
            auto s = m_messages.front();
            m_messages.pop_front();

            m_allocator.destroy(s);
            m_allocator.deallocate(s, 1);
        }

        if (!m_messages.empty())
        {
            // std::cout << "pre buffer size: " << m_messages.front()->size() << std::endl;
            do_write();
        }
        else
        {
            // 所有缓冲区的消息已经处理完毕，需要通知外部继续处理
            if (m_on_events.has<tcp::on_write_completed>("on_write_completed"))
            {
                m_on_events.get<tcp::on_write_completed>("on_write_completed")();
            }
        }
    }
    else
    {
        std::cout << "handle_write error, message: " << err.message() << std::endl;
        handle_close();

        if (m_on_events.has<tcp::on_connection_lost>("on_connection_lost"))
        {
            m_on_events.get<tcp::on_connection_lost>("on_connection_lost")(shared_from_this(), err);
        }
    }
}

void TcpTransport::handle_close()
{
    m_transport_status = transport::EN_CLOSE;
    m_socket->close();
    std::cout << "close_socket" << std::endl;
    if (m_on_events.has<tcp::on_disconnected>("on_disconnected"))
    {
        m_on_events.get<tcp::on_disconnected>("on_disconnected")();
    }
}

void TcpTransport::do_write()
{
    boost::asio::async_write(
                *m_socket,
                boost::asio::buffer(m_messages.front()->data(), m_messages.front()->size()),
                boost::asio::transfer_at_least(m_messages.front()->size()),
                boost::bind(&TcpTransport::handle_write, shared_from_this(), _1, _2));
}

void TcpTransport::do_read()
{
    boost::asio::async_read(
                *m_socket,
                boost::asio::buffer(m_read_data, m_read_data_length),
                boost::asio::transfer_at_least(1),
                boost::bind(&TcpTransport::handle_read, shared_from_this(), _1, _2)
                );
}

void TcpTransport::check_deadline()
{

}
