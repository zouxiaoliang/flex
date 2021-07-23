#ifndef BASEPROTOCOL_H
#define BASEPROTOCOL_H

#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/chrono.hpp>
#include <boost/chrono/duration.hpp>
#include <boost/bind/bind.hpp>

#include "transport/TcpTransport.h"

#include <iostream>

class TcpTransport;

/**
 * @brief The CBaseProtocol class 组装业务数据基类，并将数据发送到对端
 */
class BaseProtocol
{
public:
    BaseProtocol(boost::shared_ptr<boost::asio::io_context> ioc, boost::shared_ptr<TcpTransport> transport) :
          m_ioc(ioc),
          m_transport(transport)
    {
        using namespace boost::placeholders;
        if (m_transport)
        {
            // register event handler
            m_transport->bind_handle_connected(boost::bind(&BaseProtocol::on_connected, this));
            m_transport->bind_handle_disconnected(boost::bind(&BaseProtocol::on_disconnected, this));
            m_transport->bind_handle_data_recevied(boost::bind(&BaseProtocol::on_raw_data_received, this, _1));
            m_transport->bind_handle_write_completed(boost::bind(&BaseProtocol::on_write_completed, this));
        }
    }

    virtual ~BaseProtocol() = default;

public:
    /**
     * @brief write 发送数据到对对端
     * @param message 消息
     */
    virtual void write(const std::string &message)
    {
        using namespace boost::placeholders;
        if (m_transport)
        {
            m_transport->write(message, boost::bind(&BaseProtocol::on_write_error, this, _1));
        }
    }

    /**
     * @brief close 关闭连接
     */
    void close()
    {
        if (m_transport)
        {
            m_transport->disconnect();
        }
    }

    /**
     * @brief transport_status 获取连接状态
     * @return ETransportStatus
     */
    int transport_status()
    {
        if (m_transport)
        {
            return m_transport->status();
        }
        return transport::EN_CLOSE;
    }

protected:
    /**
     * @brief on_connected 连接创建成功后业务层回调函数
     */
    virtual void on_connected() {}

    /**
     * @brief on_disconnected 连接重置后业务层回调函数
     */
    virtual void on_disconnected() {}

    /**
     * @brief on_write_error
     * @param data
     */
    virtual void on_write_error(const std::string &data) {}

    /**
     * @brief on_write_completed
     */
    virtual void on_write_completed() {}

protected:
    /**
     * @brief on_message_received 数据接收回调
     * @param message 消息
     */
    virtual void on_message_received(const std::string &message) {}

    /**
     * @brief on_raw_data_received 对收到的消息进行处理
     * @param data
     */
    virtual void on_raw_data_received(const std::string &data) {}

protected:
    boost::shared_ptr<boost::asio::io_context> m_ioc;
    boost::shared_ptr<TcpTransport> m_transport;
};

#endif // BASEPROTOCOL_H
