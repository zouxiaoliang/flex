#include "TcpListener.h"
#include "factory/ClientFactory.h"
#include "protocol/GenericProtocol.h"

#include <boost/make_shared.hpp>
#include <boost/bind/bind.hpp>

#include <iostream>

using namespace boost::placeholders;

void TcpListener::connection_made(boost::shared_ptr<BaseProtocol> session)
{
    std::cout << "accept new client" << std::endl;
}
