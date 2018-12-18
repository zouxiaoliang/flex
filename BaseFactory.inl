#include "ClientFactory.h"
#include "Transport.h"

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <iostream>


template<class ProtocolType>
boost::shared_ptr<CBaseProtocol> CBaseFactory::connect_tcp(
        const std::string &ip, short port, time_t timeout, size_t block_size)
{
    boost::asio::ip::tcp::resolver resolver(this->m_ioc);
    auto endpoints = resolver.resolve(ip, std::to_string(port));

    return build_protocol<ProtocolType>(endpoints, timeout, block_size);
}

template<class ProtocolType>
boost::shared_ptr<CBaseProtocol> CBaseFactory::build_protocol(
        const boost::asio::ip::tcp::resolver::results_type &endpoints, time_t timeout ,size_t block_size)
{
    auto transport = boost::make_shared<Transport>(this->m_ioc, timeout, block_size);
    auto protocol = boost::make_shared<ProtocolType>(this->m_ioc, transport);

    this->__build_protocolrotocol(transport, protocol);
    transport->connect(endpoints);

    return protocol;
}
