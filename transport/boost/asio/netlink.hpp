/**
 * @author zouxiaoliang
 * @date 2021/11/10
 */
#ifndef NL_PROTOCOL_H
#define NL_PROTOCOL_H

#include "endpoint.hpp"
#include <boost/asio/basic_raw_socket.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <sys/socket.h>
#include <vector>

namespace boost {
namespace asio {

class netlink {
public:
    typedef netlink_ns::nl_endpoint<netlink>          endpoint;
    typedef std::vector<endpoint>                     endpoints;
    typedef boost::asio::basic_raw_socket<netlink>    socket;

public:
    netlink() {}
    netlink(int proto) : m_proto(proto) {}

    int type() const BOOST_ASIO_NOEXCEPT {
        return SOCK_RAW | SOCK_CLOEXEC;
    }

    int protocol() const BOOST_ASIO_NOEXCEPT {
        return m_proto;
    }

    int family() const BOOST_ASIO_NOEXCEPT {
        return PF_NETLINK;
    }

private:
    int m_proto{31};
};

} // namespace asio
} // namespace boost

#endif // NL_PROTOCOL_H
