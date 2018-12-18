#include "ClientFactory.h"

void ClientFactory::__build_protocolrotocol(boost::shared_ptr<Transport> connector, boost::shared_ptr<CBaseProtocol> protocol)
{
    std::cout << "ClientFactory::__build_protocolrotocol" << std::endl;
    connector->register_callback<Transport::on_connection_lost>("on_connection_lost", boost::bind(&ClientFactory::connection_lost, this, _1, _2));
    connector->register_callback<Transport::on_connection_failed>("on_connection_failed", boost::bind(&ClientFactory::connection_failed, this, _1, _2));
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
