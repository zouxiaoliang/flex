#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>
#include <list>

#include "BaseProtocol.h"

/**
 * @brief The Protocol class 组装业务数据，并将数据发送到对端
 */
class Protocol : public CBaseProtocol
{
private:
    struct Head
    {
        uint32_t version;
        uint32_t length;
    };

public:
    Protocol(boost::asio::io_context &ioc, boost::shared_ptr<Transport> transport);

    ~Protocol();

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
    int transport_status();

protected:
    /**
     * @brief data_received 数据接收回调
     * @param message 消息
     */
    virtual void message_received(const std::string &message);

    /**
     * @brief on_connected
     */
    virtual void on_connected();

    /**
     * @brief on_disconnected
     */
    virtual void on_disconnected();

    /**
     * @brief on_data_received 对收到的消息进行处理
     * @param data
     */
    virtual void on_data_received(const std::string &data);

    /**
     * @brief on_write_error
     * @param data
     */
    virtual void on_write_error(const std::string &data);

    /**
     * @brief print
     */
    void print();

protected:

    std::string m_buffer;

    uint64_t m_error_count;
    uint64_t m_recv_count;
    boost::asio::deadline_timer m_timer;
};

#endif // CLIENT_H
