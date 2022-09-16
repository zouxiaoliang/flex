/**
 * @author zouxiaoliang
 * @date 2021/11/10
 */
#ifndef NL_ENDPOINT_H
#define NL_ENDPOINT_H

#include <boost/asio/detail/socket_types.hpp>
#include <linux/netlink.h>

namespace boost {
namespace asio {
namespace netlink_ns {

template <class Protocol>
class nl_endpoint {

public:
    typedef Protocol                              protocol_type;
    typedef boost::asio::detail::socket_addr_type data_type;

public:
    nl_endpoint() {
        m_sockaddr.nl_family = PF_NETLINK;
        m_sockaddr.nl_groups = 0;
        m_sockaddr.nl_pid    = 0;

        m_proto_id = 0;
    }

    nl_endpoint(int group, int proto, int pid = getpid()) {
        m_sockaddr.nl_family = PF_NETLINK;
        m_sockaddr.nl_groups = group;
        m_sockaddr.nl_pid    = pid;

        m_proto_id = proto;
    }

    nl_endpoint(const nl_endpoint& other) {
        m_sockaddr = other.m_sockaddr;
        m_proto_id = other.m_proto_id;
    }

    nl_endpoint& operator=(const nl_endpoint& other) {
        m_sockaddr = other.m_sockaddr;
        m_proto_id = other.m_proto_id;
        return *this;
    }

    protocol_type protocol() const {
        return protocol_type(m_proto_id);
    }

    data_type* data() const {
        return (struct sockaddr*)&m_sockaddr;
    }

    std::size_t size() const {
        return sizeof(m_sockaddr);
    }

    void resize(std::size_t) {}

    std::size_t capacity() const {
        return sizeof(sockaddr);
    }

    friend bool operator==(const nl_endpoint<Protocol>& e1, const nl_endpoint<Protocol>& e2) {
        // return e1.m_sockaddr == e2.m_sockaddr;
        return 0 == memcmp(&e1.m_sockaddr, &e2.m_sockaddr, sizeof(sockaddr_nl));
    }

    friend bool operator!=(const nl_endpoint<Protocol>& e1, const nl_endpoint<Protocol>& e2) {
        // return !(e1.m_sockaddr == e2.m_sockaddr);
        return 0 != memcmp(&e1.m_sockaddr, &e2.m_sockaddr, sizeof(sockaddr_nl));
    }

    friend bool operator<(const nl_endpoint<Protocol>& e1, const nl_endpoint<Protocol>& e2) {
        return e1.m_sockaddr < e2.m_sockaddr;
    }

    friend bool operator>(const nl_endpoint<Protocol>& e1, const nl_endpoint<Protocol>& e2) {
        return e1.m_sockaddr > e2.m_sockaddr;
    }

    friend bool operator<=(const nl_endpoint<Protocol>& e1, const nl_endpoint<Protocol>& e2) {
        return !(e2 < e1);
    }

    friend bool operator>=(const nl_endpoint<Protocol>& e1, const nl_endpoint<Protocol>& e2) {
        return !(e1 < e2);
    }

private:
    sockaddr_nl m_sockaddr;
    int32_t     m_proto_id{1};
};
} // namespace netlink_ns
} // namespace asio
} // namespace boost
#endif // NL_ENDPOINT_H
