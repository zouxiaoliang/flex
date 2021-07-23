#include "UnixSocketTransport.h"

UnixSocketTransport::UnixSocketTransport(boost::shared_ptr<boost::asio::io_context> ioc, time_t timeout, size_t block_size):
    BaseTransport(ioc, timeout, block_size)
{

}

UnixSocketTransport::~UnixSocketTransport()
{

}

void UnixSocketTransport::connect(const std::string &path)
{

}

void UnixSocketTransport::connect()
{

}

void UnixSocketTransport::disconnect()
{

}

int32_t UnixSocketTransport::status()
{
    return m_transport_status;
}

void UnixSocketTransport::connection_mode()
{

}

void UnixSocketTransport::flush()
{

}
