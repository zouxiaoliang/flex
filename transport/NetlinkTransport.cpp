/**
 * @author zouxiaoliang
 * @date 2021/11/10
 */
#include "NetlinkTransport.h"

#include <boost/bind.hpp>
#include <boost/regex.hpp>
using namespace boost::placeholders;

namespace flex {
NetlinkTransport::NetlinkTransport(
    boost::shared_ptr<boost::asio::io_context> ioc, time_t timeout, size_t block_size, int32_t group, int proto,
    int32_t pid)
    : BaseTransport(ioc, timeout, block_size), m_socket(*ioc), m_ioc(ioc), m_read_data(new char[block_size]),
      m_read_data_length(block_size), m_messages(), m_pid(pid), m_group(group), m_proto(proto) {}

NetlinkTransport::~NetlinkTransport() {
    if (!m_read_data) {
        delete[] m_read_data;
    }
}

void NetlinkTransport::connect(const std::string& path) {
    boost::smatch nl_addr;
    boost::regex  nl_pattern("nl://(\\d+):(\\d+)");
    if (!boost::regex_match(path, nl_addr, nl_pattern)) {
        if (m_fn_handle_connection_failed) {
            m_fn_handle_connection_failed(
                shared_from_this(), boost::system::errc::make_error_code(boost::system::errc::bad_address));
        }

        return;
    }

    m_endpoints = {boost::asio::netlink::endpoint(
        atoi(std::string(nl_addr[1].first, nl_addr[1].second).c_str()),
        atoi(std::string(nl_addr[2].first, nl_addr[2].second).c_str()))};

    connect();
}
#define ASYNC_CONNECT 0
void NetlinkTransport::connect() {
    for (auto& v : m_messages) {
        m_allocator.destroy(v.data);
        m_allocator.deallocate(v.data, 1);
    }

    m_messages.clear();

    m_transport_status = transport::EN_READY;

#if ASYNC_CONNECT
    boost::asio::async_connect(
        m_socket, m_endpoints,
        boost::bind(&NetlinkTransport::handle_connect, shared_from_this(), boost::asio::placeholders::error));
#else
    boost::system::error_code ec;
    m_socket.open(boost::asio::netlink(m_proto), ec);
    if (ec) {
        if (this->m_fn_handle_connection_failed) {
            m_fn_handle_connection_failed(shared_from_this(), ec);
        }
        return;
    }
    m_socket.bind(boost::asio::netlink::endpoint(m_group, m_proto), ec);
    if (ec) {
        if (this->m_fn_handle_connection_failed) {
            m_fn_handle_connection_failed(shared_from_this(), ec);
        }
        return;
    }
#endif
    m_transport_status = transport::EN_CONNECTING;
#if !ASYNC_CONNECT
    this->handle_connect(boost::system::errc::make_error_code(boost::system::errc::success));
#endif
}

void NetlinkTransport::disconnect() {
    boost::asio::post(m_strand, boost::bind(&NetlinkTransport::handle_close, shared_from_this()));
}

void NetlinkTransport::accept(const std::string& path) {}

void NetlinkTransport::connection_mode() {}

void NetlinkTransport::write(const std::string& data, const transport::on_write_failed& handle_error) {
    // std::cout << "pre post thread id: " << boost::this_thread::get_id() << std::endl;
    boost::asio::post(m_strand, [this, data, handle_error]() {
        // std::cout << "NetlinkTransport::" << __func__ << std::endl;
        // std::cout << "post, thread id: " << boost::this_thread::get_id() << std::endl;
        // 检查是否有数据正在发送，如果队列中存在数据，则表示有数据正在发送
        bool writing = !m_messages.empty();

        // 通讯状态OK且缓存队列未满，将消息加入缓存队列
        if (m_messages.size() < 10000 && transport::EN_OK == status()) {

            auto buffer = m_allocator.allocate(1);
            nlmsg_pack(buffer, data);

            // std::cout << "send message, length: " << buffer->length() << std::endl;
            m_messages.push_back({buffer, handle_error});

            // 如果push前队列为空，手动调用do_write消费队列数据
            if (!writing) {
                do_write();
            }
        } else {
            if (handle_error) {
                auto ec = boost::system::errc::make_error_code(boost::system::errc::no_buffer_space);
                handle_error(data, ec);
            }
            m_flow_statistics.increase(FlowStatistics::FlowType::ERR, data.length());
        }
    });
}

void NetlinkTransport::flush() {
    boost::asio::post(m_strand, [this]() {
        // 检查是否有数据正在发送，如果队列中存在数据，则表示有数据正在发送

        if (m_messages.empty()) {
            // 所有缓冲区的消息已经处理完毕，需要通知外部继续处理
            if (m_fn_handle_write_completed) {
                m_fn_handle_write_completed();
            }
        }
    });
}

void NetlinkTransport::handle_connect(const boost::system::error_code& err) {
    if (!err) {
        m_transport_status = transport::EN_OK;
        if (m_fn_handle_connected) {
            m_fn_handle_connected();
        }
        do_read();
    } else {
        m_transport_status = transport::EN_CLOSE;
        if (m_fn_handle_connection_failed) {
            m_fn_handle_connection_failed(shared_from_this(), err);
        }
    }
}

void NetlinkTransport::handle_read(const boost::system::error_code& err, size_t length) {
    if (!err) {
        // 处理接收的数据
        std::string message;
        if (nlmsg_unpack(m_read_data, length, message)) {
            if (m_fn_handle_data_recevied) {
                // TODO:
                m_fn_handle_data_recevied(message);
            }
        }
        // std::cout << "handle read, length: " << length << std::endl;
        m_flow_statistics.increase(FlowStatistics::FlowType::IN, length);
        // 接收数据
        do_read();
    } else {
        handle_close();
        if (m_fn_handle_connection_lost) {
            m_fn_handle_connection_lost(shared_from_this(), err);
        }
    }
}

void NetlinkTransport::handle_write(const boost::system::error_code& err, size_t length) {
    if (!err) {
        // std::cout << "actually sends " << length << " bytes" << std::endl;

        if (!m_messages.empty()) {
            // std::cout << "buffer size: " << m_messages.front().data->size() << ", actually sends " << length << "
            // bytes"
            //           << std::endl;
            if (m_messages.front().data->size() != length) {
                std::cout << "== error: " << m_messages.front().data->size() << " != " << length << std::endl;
                return;
            }
            auto s = m_messages.front();
            m_messages.pop_front();

            m_allocator.destroy(s.data);
            m_allocator.deallocate(s.data, 1);
        }

        m_flow_statistics.increase(FlowStatistics::FlowType::OUT, length);

        if (!m_messages.empty()) {
            // std::cout << "pre buffer size: " << m_messages.front().data->size() << std::endl;
            do_write();
        } else {
            // 所有缓冲区的消息已经处理完毕，需要通知外部继续处理
            if (m_fn_handle_write_completed) {
                m_fn_handle_write_completed();
            }
        }
    } else {
        if (!m_messages.empty()) {
            const auto& msg = m_messages.front();
            if (msg.on) {
                msg.on(*msg.data, err);
            } else {
                if (m_fn_handle_write_failed) {
                    m_fn_handle_write_failed(*msg.data, err);
                }
            }
        }

        std::cout << "handle_write error, message: " << err.message() << std::endl;
        handle_close();

        if (m_fn_handle_connection_lost) {
            m_fn_handle_connection_lost(shared_from_this(), err);
        }

        m_flow_statistics.increase(FlowStatistics::FlowType::ERR, length);
    }
}

void NetlinkTransport::handle_close() {
    m_transport_status = transport::EN_CLOSE;
    m_socket.close();

    if (m_fn_handle_disconnected) {
        m_fn_handle_disconnected();
    }
}

void NetlinkTransport::do_write() {
#if 0
    boost::asio::async_write(
        m_socket, boost::asio::buffer(m_messages.front().data->data(), m_messages.front().data->size()),
        boost::asio::transfer_at_least(m_messages.front().data->size()),
        boost::bind(&NetlinkTransport::handle_write, shared_from_this(), _1, _2));
#else
    // std::cout << "async send buffer, len:" << m_messages.front().data->size() << std::endl;
    m_socket.async_send(
        boost::asio::buffer(m_messages.front().data->data(), m_messages.front().data->size()),
        boost::bind(&NetlinkTransport::handle_write, shared_from_this(), _1, _2));
#endif
}

#define DO_READ 1
void NetlinkTransport::do_read() {
#if DO_READ
#if 0
    boost::asio::async_read(
        m_socket, boost::asio::buffer(m_read_data, m_read_data_length), boost::asio::transfer_at_least(1),
        boost::bind(&NetlinkTransport::handle_read, shared_from_this(), _1, _2));
#else
    m_socket.async_receive(
        boost::asio::buffer(m_read_data, m_read_data_length),
        boost::bind(&NetlinkTransport::handle_read, shared_from_this(), _1, _2));
#endif
#endif
}

#define AUTO_MSG_ENTITY 1

void NetlinkTransport::nlmsg_pack(std::string*& buffer, const std::string& data) {
    // std::cout << "NetlinkTransport::" << __func__ << std::endl;
    auto buffer_length =
#if !AUTO_MSG_ENTITY
        sizeof(struct msghdr) + sizeof(sockaddr_nl) + sizeof(struct iovec) +
#else
        sizeof(nlmsghdr) + data.length();
#endif

        m_allocator.construct(buffer, buffer_length, '0');

    // fill messsage
    char* ptr = const_cast<char*>(buffer->data());
    memset(ptr, 0x0, buffer_length);

#if !AUTO_MSG_ENTITY
    auto msg_entity  = reinterpret_cast<struct msghdr*>(ptr);
    auto nl_sockaddr = reinterpret_cast<sockaddr_nl*>(ptr += sizeof(struct msghdr));
    auto nl_iov      = reinterpret_cast<struct iovec*>(ptr += sizeof(sockaddr_nl));
    auto nl_msg_hdr  = reinterpret_cast<nlmsghdr*>(ptr += sizeof(struct iovec));
#else
    auto nl_msg_hdr = reinterpret_cast<nlmsghdr*>(ptr);
#endif

#if !AUTO_MSG_ENTITY
    // filn entity
    msg_entity->msg_name    = nl_sockaddr;
    msg_entity->msg_namelen = sizeof(*nl_sockaddr);
    msg_entity->msg_iov     = nl_iov;
    msg_entity->msg_iovlen  = 1;

    // fill sockaddr
    nl_sockaddr->nl_family = AF_NETLINK;
#if 1
    nl_sockaddr->nl_pid    = m_pid;
    nl_sockaddr->nl_groups = m_group;
#else
    nl_sockaddr->nl_pid    = 0;
    nl_sockaddr->nl_groups = 0;
#endif
#endif
    // fill hdr
    nl_msg_hdr->nlmsg_pid = m_pid;
    nl_msg_hdr->nlmsg_len = NLMSG_SPACE(data.length());
    nl_msg_hdr->nlmsg_flags = 0;

    // fill data
    memcpy(NLMSG_DATA(nl_msg_hdr), data.c_str(), data.length());

#if !AUTO_MSG_ENTITY
    // fill iovec
    nl_iov->iov_base = nl_msg_hdr;
    nl_iov->iov_len  = nl_msg_hdr->nlmsg_len;
#endif
}

bool NetlinkTransport::nlmsg_unpack(const char* buffer, size_t length, std::string& message) {
    // std::cout << "NetlinkTransport::" << __func__ << std::endl;

#if !AUTO_MSG_ENTITY
#if 1
    if (length < sizeof(msghdr)) {
        return false;
    }
#endif
#if 0
    size_t bytes = 0;
    for (struct nlmsghdr* nlm = (struct nlmsghdr*)buffer; NLMSG_OK(nlm, bytes); nlm = NLMSG_NEXT(nlm, bytes)) {
        auto msg_entity = reinterpret_cast<const struct msghdr*>(buffer);
        auto c_buf      = reinterpret_cast<const char*>(NLMSG_DATA(msg_entity->msg_iov->iov_base));
        auto c_buf_len  = reinterpret_cast<size_t>(NLMSG_DATA(msg_entity->msg_iov->iov_len));
    }
#else
    auto msg_entity = reinterpret_cast<const struct nlmsghdr*>(buffer);
    auto c_buf      = reinterpret_cast<const char*>(NLMSG_DATA(msg_entity->msg_iov->iov_base));
    auto c_buf_len  = reinterpret_cast<size_t>(NLMSG_DATA(msg_entity->msg_iov->iov_len));

    message.assign(c_buf, c_buf_len);
#endif
#else
    auto msg_entity = reinterpret_cast<const struct nlmsghdr*>(buffer);
    auto c_buf      = reinterpret_cast<const char*>(NLMSG_DATA(msg_entity));
    auto c_buf_len  = msg_entity->nlmsg_len;

    message.assign(c_buf, c_buf_len);
#endif
    return true;
}

} // namespace flex
