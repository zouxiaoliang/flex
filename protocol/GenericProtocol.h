#ifndef GENERICPROTOCOL_H
#define GENERICPROTOCOL_H

#include <boost/asio.hpp>
#include <list>

#include "BaseProtocol.h"

/**
 * @brief The Protocol class 组装业务数据，并将数据发送到对端
 */
class GenericProtocol : public BaseProtocol
{

protected:
    struct Head
    {
        uint32_t version;
        uint32_t length;
    };

public:
    /**
     * @brief GenericProtocol
     * @param ioc
     * @param transport
     */
    GenericProtocol(boost::shared_ptr<boost::asio::io_context> ioc, boost::shared_ptr<BaseTransport> transport);

    ~GenericProtocol();

public:
    /**
     * @brief write 发送数据到对对端
     * @param message 消息
     */
    virtual void write(const std::string &message);

    /**
     * @brief close 关闭连接
     */
    void close();

    /**
     * @brief transport_status
     * @return
     */
    int32_t transport_status();

protected:
    /**
     * @brief on_message_received 数据接收回调
     * @param message 消息
     */
    virtual void on_message_received(const std::string &message);

    /**
     * @brief on_connected
     */
    virtual void on_connected();

    /**
     * @brief on_disconnected
     */
    virtual void on_disconnected();

    /**
     * @brief on_raw_data_received 对收到的消息进行处理
     * @param data
     */
    virtual void on_raw_data_received(const std::string &data);

    /**
     * @brief on_write_error
     * @param data
     */
    virtual void on_write_error(const std::string &data, const boost::system::error_code &ec);

    /**
     * @brief print
     */
    void print();

protected:

    /// @brief 接收缓冲区
    std::string m_buffer;

    /// @brief 定时器
    boost::asio::deadline_timer m_timer;
};

#endif // GENERICPROTOCOL_H
