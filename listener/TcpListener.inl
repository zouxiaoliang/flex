#include "TcpListener.h"
#include "factory/Connector.h"

#include <iostream>

template <class ProtocolType>
void TcpListener::listen(
    boost::shared_ptr<boost::asio::io_context> ioc,
    const boost::asio::ip::tcp::endpoint&      endpoint,
    boost::shared_ptr<Connector>               factory) {
    boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor = boost::make_shared<boost::asio::ip::tcp::acceptor>(*ioc);
    acceptor->open(endpoint.protocol());
    acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(1));
    acceptor->bind(endpoint);
    acceptor->listen();
    do_accept<ProtocolType>(ioc, acceptor, factory);
}

template <class ProtocolType>
void TcpListener::do_accept(
    boost::shared_ptr<boost::asio::io_context>        ioc,
    boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor,
    boost::shared_ptr<Connector>                      factory) {
    boost::shared_ptr<boost::asio::ip::tcp::socket> socket = boost::make_shared<boost::asio::ip::tcp::socket>(*ioc);
    acceptor->async_accept(*socket,
                           boost::bind(&TcpListener::handle_accept<ProtocolType>,
                                       this,
                                       ioc,
                                       acceptor,
                                       socket,
                                       factory,
                                       boost::asio::placeholders::error
                                       )
                           );
}

template <class ProtocolType>
void TcpListener::handle_accept(
    boost::shared_ptr<boost::asio::io_context>            ioc,
    boost::shared_ptr<boost::asio::ip::tcp::acceptor>     acceptor,
    const boost::shared_ptr<boost::asio::ip::tcp::socket> socket,
    boost::shared_ptr<Connector>                          factory,
    const boost::system::error_code&                      err) {
    if (!err)
    {
        do_accept<ProtocolType>(ioc ,acceptor, factory);
        connection_made(factory->on_accept<ProtocolType>(socket, 10, 1024));
    }
    else
    {
        std::cout << "eccept error: " << err.message() << std::endl;
    }
}
