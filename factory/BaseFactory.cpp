#include "BaseFactory.h"

BaseFactory::BaseFactory(boost::asio::io_context &ioc):
    m_ioc(ioc)
{

}

BaseFactory::~BaseFactory() = default;
