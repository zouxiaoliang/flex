#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

class BaseProtocol;
class BaseTransport;

/**
 * @brief The CBaseFactory class
 * @details 协议工厂，用于创建连接
 */
class Connector {
public:
    explicit Connector(boost::shared_ptr<boost::asio::io_context> ioc);

    virtual ~Connector();

public:

    /**
     * @brief connect_tcp 连接服务端
     * @param url 统一资源定位符
     * @param timeout 超时时间
     * @param block_size 消息块大小
     * @return 协议处理对象
     */

    template <class ProtocolType, class TransportType, class... Args>
    boost::shared_ptr<ProtocolType> connect(const std::string& url, Args... args);

protected:

    /**
     * @brief __build_protocol 自定义构建方法
     * @param connector
     */
    virtual void __build_protocol(boost::shared_ptr<BaseTransport> connector, boost::shared_ptr<BaseProtocol> protocol) {}

protected:
    boost::shared_ptr<boost::asio::io_context> m_ioc;
};

#include "Connector.inl"

#endif // CONNECTOR_H
