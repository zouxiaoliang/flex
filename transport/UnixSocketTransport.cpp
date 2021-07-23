#include "UnixSocketTransport.h"

UnixSocketTransport::UnixSocketTransport(boost::shared_ptr<boost::asio::io_context> ioc, time_t timeout, size_t block_size):
    BaseTransport(ioc, timeout, block_size)
{

}

UnixSocketTransport::~UnixSocketTransport()
{

}
