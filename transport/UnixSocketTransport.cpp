#include "UnixSocketTransport.h"

UnixSocketTransport::UnixSocketTransport(boost::shared_ptr<boost::asio::io_context> ioc, time_t timeout, size_t block_size):
    BaseTransport<boost::asio::local::stream_protocol::socket, unix::TOnEvent> (ioc, timeout, block_size)
{

}

UnixSocketTransport::~UnixSocketTransport()
{

}
