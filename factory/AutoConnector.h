#ifndef AUTOCONNECTOR_H
#define AUTOCONNECTOR_H

#include "Connector.h"

#include <boost/function.hpp>

/**
 * @brief The AutoReconnectFactory class
 * @details
 *      1、协议工厂，用于创建连接, 以及断开连接自动重连
 *      2、针对连接失败进行处理，例如重连机制
 *      3、针对连接丢失进行处理，例如重连机制
 */
class AutoReconnector : public Connector {
public:
    using ON_CONNECTION_LOST = boost::function<void (boost::shared_ptr<BaseTransport>, const boost::system::error_code)>;
    using ON_CONNECTION_FAILED =
        boost::function<void(boost::shared_ptr<BaseTransport>, const boost::system::error_code)>;

public:
    AutoReconnector(boost::shared_ptr<boost::asio::io_context> ioc, time_t reconnection_cycle = 5);

    virtual ~AutoReconnector();

protected:
    /**
     * @brief __build_protocol 自定义构建方法，注册重连回调函数
     * @param transport
     * @param protocol
     */
    virtual void __build_protocol(boost::shared_ptr<BaseTransport> transport, boost::shared_ptr<BaseProtocol> protocol);
public:

    /**
     * @brief bind_handle_connection_lost 绑定连接丢失事件
     * @param on 事件处理函数
     */
    inline void bind_handle_connection_lost(ON_CONNECTION_LOST on)
    {
        m_fn_connection_lost = on;
    }

    /**
     * @brief bind_handle_connection_failed 绑定连接失败事件
     * @param on 事件处理函数
     */
    inline void bind_handle_connection_failed(ON_CONNECTION_FAILED on)
    {
        m_fn_connection_failed = on;
    }

    /**
     * @brief connection_lost 当连接在半途中出现问题的时候对连接进行处理
     * @param connector 连接器
     * @param err 错误码
     */
    virtual void connection_lost(boost::shared_ptr<BaseTransport> connector, const boost::system::error_code &err);

    /**
     * @brief connection_failed 当建立连接失败后，进行处理
     * @param connector 连接器
     * @param err 错误码
     */
    virtual void connection_failed(boost::shared_ptr<BaseTransport> connector, const boost::system::error_code &err);

protected:
    ON_CONNECTION_LOST m_fn_connection_lost;
    ON_CONNECTION_FAILED m_fn_connection_failed;

    time_t m_reconnection_cycle{5};
};

#endif // AUTOCONNECTOR_H
