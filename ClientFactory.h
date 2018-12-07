#ifndef CLIENTFACTORY_H
#define CLIENTFACTORY_H

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

class Protocol;
class Transport;

/**
 * @brief The ClientFactory class
 * @details
 *      1、协议工厂，用于创建连接
 *      2、针对连接失败进行处理，例如重连机制
 *      3、针对连接丢失进行处理，例如重连机制
 */
class ClientFactory
{
public:
    ClientFactory(boost::asio::io_context &ioc);
    virtual ~ClientFactory();

    /**
     * @brief connect_tcp 连接服务端
     * @param ip 服务端ip
     * @param port 服务端端口
     * @param timeout 超时时间
     * @param block_size 消息块大小
     * @return 协议处理对象
     */
    boost::shared_ptr<Protocol> connect_tcp(const std::string &ip, short port, time_t timeout, size_t block_size);

    /**
     * @brief build_protocal 创建协议对象
     * @param endpoints 对端信息
     * @param timeout 超时时间
     * @param block_size 消息块大小
     * @return 协议处理对象
     */
    virtual boost::shared_ptr<Protocol> build_protocal(const boost::asio::ip::tcp::resolver::results_type &endpoints, time_t timeout, size_t block_size);

    /**
     * @brief connection_lost 当连接在半途中出现问题的时候对连接进行处理
     * @param connector 连接器
     * @param err 错误码
     */
    virtual void connection_lost(boost::shared_ptr<Transport> connector, const boost::system::error_code &err);

    /**
     * @brief connection_failed 当建立连接失败后，进行处理
     * @param connector 连接器
     * @param err 错误码
     */
    virtual void connection_failed(boost::shared_ptr<Transport> connector, const boost::system::error_code &err);

private:
    boost::asio::io_context &m_ioc;
};

#endif // CLIENTFACTORY_H
