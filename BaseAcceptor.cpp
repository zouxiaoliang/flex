#include "BaseAcceptor.h"
#include "ClientFactory.h"
#include "Protocol.h"

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

#include <iostream>

CAcceptor::CAcceptor(boost::asio::io_context &ioc, boost::shared_ptr<CBaseFactory> factory):
    m_ioc(ioc),
    m_acceptor(ioc),
    m_factory(factory)
{
}

CAcceptor::~CAcceptor()
{

}

void CAcceptor::listen(const boost::asio::ip::tcp::endpoint &endpoint)
{
    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(1));
    m_acceptor.bind(endpoint);
    m_acceptor.listen();
    do_accept();
}

void CAcceptor::do_accept()
{
    boost::shared_ptr<boost::asio::ip::tcp::socket> socket = boost::make_shared<boost::asio::ip::tcp::socket>(m_ioc);
    m_acceptor.async_accept(*socket,
                            boost::bind(&CAcceptor::handle_accept,
                                        this,
                                        socket,
                                        boost::asio::placeholders::error)
                            );
}

void CAcceptor::handle_accept(const boost::shared_ptr<boost::asio::ip::tcp::socket> session, const boost::system::error_code &err)
{
    do_accept();

    connection_made(session, err);
}

void CAcceptor::connection_made(const boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code &err)
{
    if(!err)
    {
        // boost::system::error_code endpoint_error;
        // socket->local_endpoint(endpoint_error);
        // socket->remote_endpoint(endpoint_error);
        std::cout << "eccept new client" << std::endl;
    }
    else
    {
        std::cout << "eccept error: " << err.message() << std::endl;
    }
}

void CAcceptorV2::connection_made(boost::shared_ptr<CBaseProtocol> session)
{
    std::cout << "eccept new client" << std::endl;
}
