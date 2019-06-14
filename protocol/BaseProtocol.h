#ifndef BASEPROTOCOL_H
#define BASEPROTOCOL_H

#include <boost/asio.hpp>


class TcpTransport;

/**
 * @brief The CBaseProtocol class 组装业务数据基类，并将数据发送到对端
 */
class BaseProtocol
{
public:
    BaseProtocol(boost::asio::io_context &ioc, boost::shared_ptr<TcpTransport> transport);

    virtual ~BaseProtocol();

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
     * @brief transport_status 获取连接状态
     * @return  EN_READY
                EN_OK
                EN_CLOSE
     */
    int transport_status();

protected:
    /**
     * @brief on_connected 连接创建成功后业务层回调函数
     */
    virtual void on_connected();

    /**
     * @brief on_disconnected 连接重置后业务层回调函数
     */
    virtual void on_disconnected();

    /**
     * @brief on_write_error
     * @param data
     */
    virtual void on_write_error(const std::string &data);

protected:
    /**
     * @brief on_message_received 数据接收回调
     * @param message 消息
     */
    virtual void on_message_received(const std::string &message);

    /**
     * @brief on_raw_data_received 对收到的消息进行处理
     * @param data
     */
    virtual void on_raw_data_received(const std::string &data);

protected:
    boost::asio::io_context &m_ioc;
    boost::shared_ptr<TcpTransport> m_transport;
};

#endif // BASEPROTOCOL_H
