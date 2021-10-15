#ifndef UNIXSOCKETTRANSPORT_H
#define UNIXSOCKETTRANSPORT_H

#include "BaseTransport.h"
#include <boost/asio.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/enable_shared_from_this.hpp>

class UnixSocketTransport : public boost::enable_shared_from_this<UnixSocketTransport>,
                            public BaseTransport {
public:
    /**
     * @brief UnixSocketTransport
     */
    UnixSocketTransport(boost::shared_ptr<boost::asio::io_context> ioc,
                        time_t timeout,
                        size_t block_size);

    /**
     * @brief ~UnixSocketTransport
     */
    virtual ~UnixSocketTransport();

    /**
     * @brief connect
     * @param path
     */
    void connect(const std::string &path) override;

    /**
     * @brief connect
     */
    void connect() override;

    /**
     * @brief disconnect
     */
    void disconnect() override;

    /**
     * @brief status
     * @return
     */
    int32_t status() override;

    /**
     * @brief connection_mode
     */
    void connection_mode() override;

    /**
     * @brief flush
     */
    void flush() override;

protected:
    boost::asio::local::stream_protocol::socket   m_unix_socket;
    boost::asio::local::stream_protocol::endpoint m_endpoint;
};

#endif // UNIXSOCKETTRANSPORT_H
