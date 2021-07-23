#ifndef UNIXSOCKETTRANSPORT_H
#define UNIXSOCKETTRANSPORT_H

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "BaseTransport.h"

#include "utils/KeyVariant.h"
#include "utils/sgi_plus.h"

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
};

#endif // UNIXSOCKETTRANSPORT_H
