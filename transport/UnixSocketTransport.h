#ifndef UNIXSOCKETTRANSPORT_H
#define UNIXSOCKETTRANSPORT_H

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "BaseTransport.h"

#include "utils/KeyVariant.h"
#include "utils/sgi_plus.h"

class UnixSocketTransport;

namespace unix_socket {

/**
 * @brief CallBack 可变类型回调函数
 */
typedef boost::function<void ()> on_connected;
typedef boost::function<void ()> on_disconnected;
typedef boost::function<void(const std::string &)> on_data_recevied;
typedef boost::function<void(boost::shared_ptr<UnixSocketTransport>, const boost::system::error_code&)> on_connection_lost;
typedef boost::function<void(boost::shared_ptr<UnixSocketTransport>, const boost::system::error_code&)> on_connection_failed;

typedef KeyVariant<
    boost::function<void()>,
    boost::function<void(const std::string &)>,
    boost::function<void(boost::shared_ptr<UnixSocketTransport>, const boost::system::error_code&)>
> TOnEvent;

}

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
};

#endif // UNIXSOCKETTRANSPORT_H
