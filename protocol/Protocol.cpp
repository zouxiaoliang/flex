#include "Protocol.h"
#include "transport/TcpTransport.h"

#include <boost/make_shared.hpp>
#include <boost/chrono.hpp>
#include <boost/chrono/duration.hpp>
#include <boost/bind.hpp>

#include <arpa/inet.h>

#include <iostream>

Protocol::Protocol(boost::asio::io_context &ioc, boost::shared_ptr<TcpTransport> transport):
    CBaseProtocol(ioc, transport),
    m_timer(ioc)
{
    m_error_count = 0;
    m_recv_count = 0;
    m_timer.expires_from_now(boost::posix_time::seconds(5));
    m_timer.async_wait(boost::bind(&Protocol::print, this));
    m_buffer.reserve(100 * 1024);
}

Protocol::~Protocol()
{

}

void Protocol::write(const std::string &message)
{
    if (m_transport && m_transport->status() == TcpTransport::EN_OK)
    {
        std::string buffer;
        buffer.resize(sizeof(Head) + message.length());

        Head *h = (Head *)buffer.c_str();
        h->version = 0xffffffff;
        h->length = ntohl(message.length());

        char *body = (char *)h + sizeof(Head);
        memcpy(body, message.c_str(), message.size());

        m_transport->write(buffer, boost::bind(&Protocol::on_write_error, this, _1));
    }
}

void Protocol::close()
{
    if (m_transport)
    {
        m_transport->disconnect();
    }
}

int Protocol::transport_status()
{
    if (m_transport)
    {
        return m_transport->status();
    }

    return TcpTransport::EN_CLOSE;
}

void Protocol::message_received(const std::string &message)
{
    // std::cout << "recv: " << message << std::endl;
    ++m_recv_count;
}

void Protocol::on_connected()
{
    m_buffer.clear();
    // 连接成功，需要做的事情
    // 发送登陆请求
    this->write("hello world");
}

void Protocol::on_disconnected()
{
    m_buffer.clear();
}

void Protocol::on_data_received(const std::string &data)
{
    // std::cout << "recv data length: " << data.size() << std::endl;
    // return ;
    // 处理接收到的数据
    m_buffer.append(data);

    size_t pos = 0;
    while (true)
    {
        if (m_buffer.size() < sizeof(Head))
            break ;

        Head *h = (Head*)&m_buffer[pos];
        if ((m_buffer.size() - pos) < (sizeof(Head) + ntohl(h->length)))
            break ;

        // 对收到的消息包进行处理
        // 对登陆应答进行处理
        // 对心跳应答进行处理
        message_received(std::string(&m_buffer[pos + sizeof(Head)], ntohl(h->length)));

        pos += sizeof(Head) + ntohl(h->length);
    }
    m_buffer = m_buffer.substr(pos);
}

void Protocol::on_write_error(const std::string &data)
{
//    std::cout << "send data failed, data length: " << data.size() <<
    //                 ", transport status: " << m_transport->status() << std::endl;
    ++ m_error_count;
}

void Protocol::print()
{
    std::cout << "time:" << ::time(0) << std::endl;
    std::cout << "      error_count: " << m_error_count << std::endl;
    std::cout << "      recv_count: " << m_recv_count << std::endl;

    m_timer.expires_from_now(boost::posix_time::seconds(5));
    m_timer.async_wait(boost::bind(&Protocol::print, this));
}
