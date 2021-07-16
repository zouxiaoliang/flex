#include "ClientFactory.h"
#include "transport/TcpTransport.h"

#include <boost/make_shared.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>

using namespace boost::placeholders;


template<class ProtocolType, class TransportType>
boost::shared_ptr<ProtocolType> BaseFactory::connect_tcp(const std::string &ip, int32_t port, time_t timeout, size_t block_size)
{
    boost::system::error_code ec;
    boost::asio::ip::tcp::resolver resolver(*this->m_ioc);
    auto endpoints = resolver.resolve(ip, std::to_string(port), ec);

    if (ec)
    {
        return nullptr;
    }
    return build_protocol<ProtocolType, TransportType>(endpoints, timeout, block_size);
}

template<class ProtocolType, class TransportType>
boost::shared_ptr<ProtocolType> BaseFactory::build_protocol(
        const boost::asio::ip::tcp::resolver::results_type &endpoints, time_t timeout ,size_t block_size)
{
    auto socket = boost::make_shared<boost::asio::ip::tcp::socket>(*this->m_ioc);
    auto transport = boost::make_shared<TransportType>(this->m_ioc, socket, timeout, block_size);
    auto protocol = boost::make_shared<ProtocolType>(this->m_ioc, transport);

    this->__build_protocol(transport, protocol);
    transport->connect(endpoints);

    return protocol;
}

template<class ProtocolType>
boost::shared_ptr<ProtocolType> BaseFactory::on_accept(const boost::shared_ptr<boost::asio::ip::tcp::socket> socket, time_t timeout ,size_t block_size)
{
    auto transport = boost::make_shared<TcpTransport>(this->m_ioc, socket, timeout, block_size);
    auto protocol = boost::make_shared<ProtocolType>(this->m_ioc, transport);

    this->__build_protocol(transport, protocol);
    transport->connection_made();

    return protocol;
}

