#ifndef CLIENTFACTORY_H
#define CLIENTFACTORY_H


#include "BaseFactory.h"

/**
 * @brief The ClientFactory class
 * @details
 *      1、协议工厂，用于创建连接
 *      2、针对连接失败进行处理，例如重连机制
 *      3、针对连接丢失进行处理，例如重连机制
 */
class ClientFactory : public BaseFactory
{
public:

    ClientFactory(boost::asio::io_context &ioc);

    virtual ~ClientFactory();
protected:

    /**
     * @brief __build_protocol 自定义构建方法，注册重连回调函数
     * @param transport
     * @param protocol
     */
    virtual void __build_protocol(boost::shared_ptr<TcpTransport> transport, boost::shared_ptr<BaseProtocol> protocol);
public:

    /**
     * @brief connection_lost 当连接在半途中出现问题的时候对连接进行处理
     * @param connector 连接器
     * @param err 错误码
     */
    virtual void connection_lost(boost::shared_ptr<TcpTransport> connector, const boost::system::error_code &err);

    /**
     * @brief connection_failed 当建立连接失败后，进行处理
     * @param connector 连接器
     * @param err 错误码
     */
    virtual void connection_failed(boost::shared_ptr<TcpTransport> connector, const boost::system::error_code &err);
};

#endif // CLIENTFACTORY_H
