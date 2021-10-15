#include "Connector.h"

Connector::Connector(boost::shared_ptr<boost::asio::io_context> ioc) : m_ioc(ioc) {}

Connector::~Connector() = default;
