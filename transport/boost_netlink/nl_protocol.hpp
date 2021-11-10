/**
 * @author zouxiaoliang
 * @date 2021/11/10
 */
#ifndef NL_PROTOCOL_H
#define NL_PROTOCOL_H

#include "nl_endpoint.hpp"
#include <boost/asio/basic_raw_socket.hpp>
#include <sys/socket.h>

namespace boost {
namespace asio {
namespace netlink {

class nl_protocol {
public:
    typedef nl_endpoint<nl_protocol>                   endpoint;
    typedef boost::asio::basic_raw_socket<nl_protocol> socket;

public:
    nl_protocol();
    nl_protocol(int proto) : m_proto(proto) {}

    int type() const {
        return SOCK_RAW;
    }

    int protocol() const {
        return m_proto;
    }

    int family() const {
        return PF_NETLINK;
    }

private:
    int m_proto{0};
};

} // namespace netlink
} // namespace asio
} // namespace boost

#endif // NL_PROTOCOL_H
