#ifndef BASETRANSPORT_H
#define BASETRANSPORT_H

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/atomic.hpp>
#include <string>

#include "utils/KeyVariant.h"

namespace transport {
/**
 * transport 状态
 */
enum
{
    EN_READY,
    EN_CONNECTING,
    EN_OK,
    EN_CLOSE
};
}

class BaseProtocol;

template<class EndpointType, class OnEvent>
class BaseTransport
{
public:
    BaseTransport(boost::asio::io_context &ioc,
                  time_t timeout,
                  size_t block_size) :
        m_strand(ioc), m_timeout(timeout), m_block_size(block_size), m_transport_status(transport::EN_CLOSE) {}

    virtual ~BaseTransport() = default;

    /**
     * @brief set_protocol
     * @param protocol
     */
    void set_protocol(boost::shared_ptr<BaseProtocol> protocol) { m_protocol = protocol; }

    /**
     * @brief protocol
     * @return
     */
    boost::shared_ptr<BaseProtocol> protocol() { return m_protocol; }

    /**
     * @brief connect
     * @param endpoionts
     */
    virtual void connect(const EndpointType &) {}

    /**
     * @brief connect
     */
    virtual void connect() {}

    /**
     * @brief disconnect
     */
    virtual void disconnect() {}

    /**
     * @brief status
     * @return
     */
    virtual int32_t status() { return m_transport_status; }

    /**
     * @brief connection_mode
     */
    virtual void connection_mode() {}

    /**
     * @brief write
     * @param data
     * @param handle_error
     */
    void write(const std::string &data, boost::function<void(const std::string&)> handle_error = nullptr) {}

    /**
     * @brief set_on_read
     * @param on_data_recevied
     */
    void set_on_read(boost::function<void(const std::string &data)> on_read) { m_on_read = on_read; }

    /**
     * @brief register_callback 注册事件回调函数
     * @param name
     * @param callback
     * @return
     */
    template<class T>
    bool register_callback(const char *name, T callback)
    {
        if (nullptr == name)
        {
            return false;
        }
        std::cout << "register callback name: " << name << std::endl;
        m_on_events.template set<T>(name, callback);
        return true;
    }

    /**
     * @brief unregister_callback 注销事件回调函数
     * @param name
     */
    void unregister_callback(const char *name)
    {
        if (nullptr == name)
        {
            return;
        }
        m_on_events.remove(name);
    }

protected:
    boost::asio::io_context::strand m_strand;
    time_t m_timeout;
    size_t m_block_size;

    boost::atomic_int32_t m_transport_status;
    boost::function<void(const std::string &data)> m_on_read;

    boost::shared_ptr<BaseProtocol> m_protocol;
    OnEvent m_on_events;
};


#endif //BASETRANSPORT_H
