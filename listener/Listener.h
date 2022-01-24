#ifndef LISTENER_H
#define LISTENER_H

#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include <list>

class BaseProtocol;
class BaseTransport;

class Listener {
public:
    explicit Listener(boost::shared_ptr<boost::asio::io_context> ioc) : m_ioc(ioc) {}
    virtual ~Listener() = default;

public:
    template <class ProtocolType, class TransportType, class... Args>
    void listen(const std::string& url, Args... args) {
        auto builder = [=](Args... builder_args) -> boost::shared_ptr<TransportType> {
            auto transport = boost::make_shared<TransportType>(this->m_ioc, args...);
            auto protocol  = boost::make_shared<ProtocolType>(this->m_ioc, transport);

            this->__build_protocol(transport, protocol);

            return transport;
        };

        auto on_accept = [=](boost::shared_ptr<BaseTransport> transport) -> void {
            auto protocol = boost::make_shared<ProtocolType>(this->m_ioc, transport);
            this->__build_protocol(transport, protocol);
        };
        auto transport = builder(args...);
        transport->accept(url, on_accept);
    }

protected:
    virtual void
    __build_protocol(boost::shared_ptr<BaseTransport> transport, boost::shared_ptr<BaseProtocol> protocol) {
        // connection object mamanger
        m_protocols.push_back(protocol);
    }

protected:
    boost::shared_ptr<boost::asio::io_context> m_ioc;
    std::list<boost::shared_ptr<BaseProtocol>> m_protocols;
};

#endif // LISTENER_H
