#include "AutoConnector.h"
#include "transport/TcpTransport.h"

#include <boost/make_shared.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>

using namespace boost::placeholders;

template <class ProtocolType, class TransportType, class... Args>
boost::shared_ptr<ProtocolType> Connector::connect(const std::string& url, Args... args) {
    auto transport = boost::make_shared<TransportType>(this->m_ioc, args...);
    auto protocol  = boost::make_shared<ProtocolType>(this->m_ioc, transport);

    this->__build_protocol(transport, protocol);
    transport->connect(url);

    return protocol;
}
