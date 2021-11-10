/**
 * @author zouxiaoliang
 * @date 2021/11/10
 */
#include "NetlinkTransport.h"

#include <boost/bind.hpp>
#include <boost/regex.hpp>
using namespace boost::placeholders;

NetlinkTransport::NetlinkTransport(boost::shared_ptr<boost::asio::io_context> ioc, time_t timeout, size_t block_size)
    : BaseTransport(ioc, timeout, block_size), m_socket(*ioc), m_ioc(ioc), m_read_data(new char[block_size]),
      m_read_data_length(block_size), m_messages() {}

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

    m_endpoint = boost::asio::netlink::nl_protocol::endpoint(
        atoi(std::string(nl_addr[1].first, nl_addr[1].second).c_str()),
        atoi(std::string(nl_addr[2].first, nl_addr[2].second).c_str()));

    connect();
}

void NetlinkTransport::connect() {
    for (auto& v : m_messages) {
        m_allocator.destroy(v.data);
        m_allocator.deallocate(v.data, 1);
    }

    m_messages.clear();

    m_transport_status = transport::EN_READY;

    boost::asio::async_connect(
        m_socket, m_endpoint,
        boost::bind(&NetlinkTransport::handle_connect, shared_from_this(), boost::asio::placeholders::error));

    m_transport_status = transport::EN_CONNECTING;
}

void NetlinkTransport::disconnect() {
    boost::asio::post(m_strand, boost::bind(&NetlinkTransport::handle_close, shared_from_this()));
}

void NetlinkTransport::accept(const std::string& path) {}

void NetlinkTransport::connection_mode() {}

void NetlinkTransport::write(const std::string& data, const transport::on_write_failed& handle_error) {
    // std::cout << "pre post thread id: " << boost::this_thread::get_id() << std::endl;
    boost::asio::post(m_strand, [this, data, handle_error]() {
        // std::cout << "post, thread id: " << boost::this_thread::get_id() << std::endl;
        // 检查是否有数据正在发送，如果队列中存在数据，则表示有数据正在发送
        bool writing = !m_messages.empty();

        // 通讯状态OK且缓存队列未满，将消息加入缓存队列
        if (m_messages.size() < 10000 && transport::EN_OK == status()) {
            auto s = m_allocator.allocate(1);
            m_allocator.construct(s, data);
            // TODO:
            m_messages.push_back({s, handle_error});

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
        if (m_fn_handle_data_recevied) {
            // TODO:
            m_fn_handle_data_recevied(std::string(m_read_data, length));
        }
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
            // std::cout << "buffer size: " << m_messages.front()->size() <<  ", actually sends " << length << " bytes"
            // << std::endl;
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
            // std::cout << "pre buffer size: " << m_messages.front()->size() << std::endl;
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
    boost::asio::async_write(
        m_socket, boost::asio::buffer(m_messages.front().data->data(), m_messages.front().data->size()),
        boost::asio::transfer_at_least(m_messages.front().data->size()),
        boost::bind(&NetlinkTransport::handle_write, shared_from_this(), _1, _2));
}

void NetlinkTransport::do_read() {
    boost::asio::async_read(
        m_socket, boost::asio::buffer(m_read_data, m_read_data_length), boost::asio::transfer_at_least(1),
        boost::bind(&NetlinkTransport::handle_read, shared_from_this(), _1, _2));
}
