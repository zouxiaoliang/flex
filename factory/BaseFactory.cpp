#include "BaseFactory.h"

CBaseFactory::CBaseFactory(boost::asio::io_context &ioc):
    m_ioc(ioc)
{

}

CBaseFactory::~CBaseFactory() = default;
