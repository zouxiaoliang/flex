#ifndef LISTENER_H
#define LISTENER_H

#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

class BaseProtocol;
class BaseTransport;

class Listener {
public:
    explicit Listener(boost::shared_ptr<boost::asio::io_context> ioc) : m_ioc(ioc) {}
    virtual ~Listener() = default;

public:
    template <class ProtocolType, class TransportType, class... Args>
    void listen(const std::string& url, Args... args) {
        auto transport = boost::make_shared<TransportType>(this->m_ioc, args...);
        auto protocol  = boost::make_shared<ProtocolType>(this->m_ioc, transport);

        this->__build_protocol(transport, protocol);

        transport->accept(url);
    }

protected:
    virtual void
    __build_protocol(boost::shared_ptr<BaseTransport> transport, boost::shared_ptr<BaseProtocol> protocol) {
        // connection object mamanger
    }

protected:
    boost::shared_ptr<boost::asio::io_context> m_ioc;
};

#endif // LISTENER_H
