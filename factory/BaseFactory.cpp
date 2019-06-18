#include "BaseFactory.h"

BaseFactory::BaseFactory(boost::shared_ptr<boost::asio::io_context> ioc):
    m_ioc(ioc)
{

}

BaseFactory::~BaseFactory() = default;
