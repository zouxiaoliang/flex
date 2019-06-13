#ifndef BASEFACTORY_H
#define BASEFACTORY_H


#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

class CBaseProtocol;
class Transport;
typedef Transport CBaseTransport;

/**
 * @brief The CBaseFactory class
 * @details 协议工厂，用于创建连接
 */
class CBaseFactory {
public:

    explicit CBaseFactory(boost::asio::io_context &ioc);

    virtual ~CBaseFactory();

public:

    /**
     * @brief connect_tcp 连接服务端
     * @param ip 服务端ip
     * @param port 服务端端口
     * @param timeout 超时时间
     * @param block_size 消息块大小
     * @return 协议处理对象
     */
    template<class ProtocolType>
    boost::shared_ptr<CBaseProtocol> connect_tcp(const std::string &ip, short port, time_t timeout, size_t block_size);

    template<class ProtocolType>
    boost::shared_ptr<CBaseProtocol>
    build_accept(const boost::shared_ptr<boost::asio::ip::tcp::socket> socket, time_t timeout, size_t block_size);

    /**
     * @brief build_protocol 创建协议对象
     * @param endpoints 对端信息
     * @param timeout 超时时间
     * @param block_size 消息块大小
     * @return 协议处理对象
     */
    template<class ProtocolType>
    boost::shared_ptr<CBaseProtocol> build_protocol(const boost::asio::ip::tcp::resolver::results_type &endpoints,
                                                    time_t timeout, size_t block_size);

protected:

    /**
     * @brief __build_protocol 自定义构建方法
     * @param connector
     */
    virtual void __build_protocol(boost::shared_ptr<Transport> connector, boost::shared_ptr<CBaseProtocol> protocol) {}

protected:

    boost::asio::io_context &m_ioc;
};

#include "BaseFactory.inl"

#endif // BASEFACTORY_H
