#include "ClientFactory.h"
#include "Transport.h"
#include "Protocol.h"

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <iostream>

ClientFactory::ClientFactory(boost::asio::io_context &ioc):
    m_ioc(ioc)
{

}

ClientFactory::~ClientFactory()
{

}

boost::shared_ptr<Protocol> ClientFactory::connect_tcp(
        const std::string &ip, short port, time_t timeout, size_t block_size)
{
    boost::asio::ip::tcp::resolver resolver(m_ioc);
    auto endpoints = resolver.resolve(ip, std::to_string(port));

    return build_protocal(endpoints, timeout, block_size);
}

boost::shared_ptr<Protocol> ClientFactory::build_protocal(
        const boost::asio::ip::tcp::resolver::results_type &endpoints, time_t timeout ,size_t block_size)
{
    auto transport = boost::make_shared<Transport>(m_ioc, timeout, block_size);
    auto protocol = boost::make_shared<Protocol>(m_ioc, transport);

    transport->set_on_connection_lost(boost::bind(&ClientFactory::connection_lost, this, _1, _2));
    transport->set_on_connection_failed(boost::bind(&ClientFactory::connection_failed, this, _1, _2));

    transport->connect(endpoints);

    return protocol;
}

void ClientFactory::connection_lost(boost::shared_ptr<Transport> connector, const boost::system::error_code &err)
{
    if (err)
    {
        boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(5));
        std::cout << "connection lost, retry connect to server" << std::endl;
        connector->connect();
    }
}

void ClientFactory::connection_failed(boost::shared_ptr<Transport> connector, const boost::system::error_code &err)
{
    if (err)
    {
        boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(5));
        std::cout << "connection failed, retry connect to server" << std::endl;
        connector->connect();
    }
}
