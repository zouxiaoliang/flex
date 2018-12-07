#include "Transport.h"

#include <iostream>

#include <boost/bind.hpp>
#include <boost/thread.hpp>

Transport::Transport(boost::asio::io_context &ioc, time_t timeout, size_t block_size) :
    m_strand(ioc),
    m_socket(ioc),
    m_block_size(block_size),
    m_timeout(timeout),
    m_read_data(new char[block_size]),
    m_read_data_length(block_size),
    m_write_data(),
    m_messages(),
    m_transport_status(EN_CLOSE)
{

}

Transport::~Transport()
{
    if (!m_read_data)
    {
        delete [] m_read_data;
    }
}

void Transport::set_protocol(boost::shared_ptr<Protocol> protocol)
{
    m_protocol = protocol;
}

boost::shared_ptr<Protocol> Transport::protocol()
{
    return m_protocol;
}

void Transport::connect(const boost::asio::ip::tcp::resolver::results_type &endpoints)
{
    m_endpoints = endpoints;

    connect();
}

void Transport::connect()
{
    m_write_data.clear();
    for (auto v: m_messages)
    {
        m_allocator.destroy(v);
        m_allocator.deallocate(v, 1);
    }
    m_messages.clear();

    m_transport_status = EN_READY;

    boost::asio::async_connect(
                m_socket, m_endpoints,
                boost::bind(&Transport::handle_connect, this, boost::asio::placeholders::error)
                );
    m_transport_status = EN_CONNECTING;
}

void Transport::disconnect()
{
    boost::asio::post(m_strand, boost::bind(&Transport::handle_close, shared_from_this()));
}

int32_t Transport::status()
{
    return m_transport_status;
}

void Transport::write(const std::string &data, boost::function<void(const std::string &)> handle_error)
{
    boost::asio::post(m_strand, [this, data, handle_error]() {

        bool trigger = !m_write_data.empty();
        if (m_messages.size() < 10000 && EN_OK == status())
        {
            std::string *s = m_allocator.allocate(1);
            m_allocator.construct(s, data);

            m_messages.push_back(s);
        }
        else
        {
            if (handle_error)
                handle_error(data);
        }

        if (!trigger) {
            do_write();
        }
    });
}

void Transport::set_on_data_received(boost::function<void (const std::string &)> on_data_recevied)
{
    m_on_data_recevied = on_data_recevied;
}

void Transport::set_on_connected(boost::function<void ()> on_connected)
{
    m_on_connected = on_connected;
}

void Transport::set_on_disconnected(boost::function<void ()> on_disconnected)
{
    m_on_disconnected = on_disconnected;
}

void Transport::set_on_connection_failed(boost::function<void(boost::shared_ptr<Transport>, boost::system::error_code)> on_connection_failed)
{
    m_on_connection_failed = on_connection_failed;
}

void Transport::set_on_connection_lost(boost::function<void (boost::shared_ptr<Transport>, boost::system::error_code)> on_connection_lost)
{
    m_on_connection_lost = on_connection_lost;
}

void Transport::handle_connect(const boost::system::error_code &err)
{
    if (!err)
    {
        boost::system::error_code set_option_err;
        boost::asio::ip::tcp::no_delay no_delay(true);

        m_socket.set_option(no_delay, set_option_err);
        if (!set_option_err)
        {
            m_transport_status = EN_OK;
            if (m_on_connected)
            {
                m_on_connected();
            }

            // 接收数据
            do_read();
        }
    }
    else
    {
        std::cout << "handle_connect error, messsage: " << err.message() << std::endl;
        if (m_on_connection_failed)
        {
            m_on_connection_failed(shared_from_this(), err);
        }
    }
}

void Transport::handle_read(const boost::system::error_code &err, size_t length)
{
    if(!err)
    {
        // std::cout << "handle read: " << length << std::endl;

        // 处理接收的数据
        if (m_on_data_recevied)
        {
            m_on_data_recevied(std::string(m_read_data, length));
        }

        // 接收数据
        do_read();
    }
    else
    {
        std::cout << "handle_read error, messsage: " << err.message() << std::endl;
        if (m_on_connection_lost)
        {
            m_on_connection_lost(shared_from_this(), err);
        }
    }
}

void Transport::handle_write(const boost::system::error_code &err, size_t length)
{
    if (!err)
    {
        // std::cout << "actually sends " << length << " bytes" << std::endl;

        m_write_data = m_write_data.substr(length);
        if (!m_messages.empty())
        {
            std::string *s = m_messages.front();
            m_messages.pop_front();

            m_write_data.append(*s);

            m_allocator.destroy(s);
            m_allocator.deallocate(s, 1);
        }

        if (!m_write_data.empty())
        {
            do_write();
        }
    }
    else
    {
        std::cout << "handle_write error, messsage: " << err.message() << std::endl;
        handle_close();
        if (m_on_connection_lost)
        {
            m_on_connection_lost(shared_from_this(), err);
        }
    }
}

void Transport::handle_close()
{
    m_transport_status = EN_CLOSE;
    m_socket.close();
    if (m_on_disconnected)
    {
        m_on_disconnected();
    }
    std::cout << "close_socket" << std::endl;
}

void Transport::do_write()
{
    boost::asio::async_write(
                m_socket,
                boost::asio::buffer(m_write_data.data(), m_write_data.size()),
                boost::bind(&Transport::handle_write, this, _1, _2));
}

void Transport::do_read()
{
    boost::asio::async_read(
                m_socket,
                boost::asio::buffer(m_read_data, m_read_data_length),
                boost::asio::transfer_at_least(1),
                boost::bind(&Transport::handle_read, this, _1, _2)
                );
}

