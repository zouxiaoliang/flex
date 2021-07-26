#include "ClientFactory.h"

ClientFactory::ClientFactory(boost::shared_ptr<boost::asio::io_context> ioc):
    BaseFactory(ioc)
{

}

ClientFactory::~ClientFactory()
{

}

void ClientFactory::__build_protocol(boost::shared_ptr<BaseTransport> connector, boost::shared_ptr<BaseProtocol> protocol)
{
    if (m_fn_connection_lost)
    {
        connector->bind_handle_connection_lost(m_fn_connection_lost);
    }
    else
    {
        connector->bind_handle_connection_lost(boost::bind(&ClientFactory::connection_lost, this, _1, _2));
    }

    if (m_fn_connection_failed)
    {
        connector->bind_handle_connection_failed(m_fn_connection_failed);
    }
    else
    {
        connector->bind_handle_connection_failed(boost::bind(&ClientFactory::connection_failed, this, _1, _2));
    }
}

void ClientFactory::connection_lost(boost::shared_ptr<BaseTransport> connector, const boost::system::error_code &err)
{
    if (err)
    {
        boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(5));
        std::cout << "connection lost, retry connect to server" << std::endl;
        connector->connect();
    }
}

void ClientFactory::connection_failed(boost::shared_ptr<BaseTransport> connector, const boost::system::error_code &err)
{
    if (err)
    {
        boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(5));
        std::cout << "connection failed, retry connect to server, what: " << err.message() << std::endl;
        connector->connect();
    }
}
