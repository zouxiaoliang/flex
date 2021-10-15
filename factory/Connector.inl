#include "AutoConnector.h"
#include "transport/TcpTransport.h"

#include <boost/make_shared.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>

using namespace boost::placeholders;

template <class ProtocolType, class TransportType>
boost::shared_ptr<ProtocolType> Connector::connect(const std::string& url, time_t timeout, size_t block_size) {
    auto transport = boost::make_shared<TransportType>(this->m_ioc, timeout, block_size);
    auto protocol  = boost::make_shared<ProtocolType>(this->m_ioc, transport);

    this->__build_protocol(transport, protocol);
    transport->connect(url);

    return protocol;
}

template <class ProtocolType>
boost::shared_ptr<ProtocolType>
Connector::on_accept(const boost::shared_ptr<boost::asio::ip::tcp::socket> socket, time_t timeout, size_t block_size) {
    auto transport = boost::make_shared<TcpTransport>(this->m_ioc, socket, timeout, block_size);
    auto protocol = boost::make_shared<ProtocolType>(this->m_ioc, transport);

    this->__build_protocol(transport, protocol);
    transport->connection_made();

    return protocol;
}
