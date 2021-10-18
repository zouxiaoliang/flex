#include "TcpTransport.h"

#include <iostream>

#include <boost/bind/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>

using namespace boost::placeholders;

TcpTransport::TcpTransport(boost::shared_ptr<boost::asio::io_context> ioc, time_t timeout, size_t block_size)
    : BaseTransport(ioc, timeout, block_size), m_socket(*ioc), m_resolver(*ioc), m_ioc(ioc),
      m_read_data(new char[block_size]), m_read_data_length(block_size), m_messages() {}

TcpTransport::~TcpTransport() {
    if (!m_read_data) {
        delete[] m_read_data;
    }
}

void TcpTransport::connect(const std::string& path) {
    boost::smatch hosts;
    boost::regex  tcp_pattern("tcp://(.*):(\\d+)");
    if (!boost::regex_match(path, hosts, tcp_pattern)) {
        if (m_fn_handle_connection_failed) {
            m_fn_handle_connection_failed(
                shared_from_this(), boost::system::errc::make_error_code(boost::system::errc::bad_address));
        }

        return;
    }

    /*
    std::cout << "host: " << std::string(hosts[0].first, hosts[0].second) << std::endl;
    std::cout << "port: " << std::string(hosts[1].first, hosts[1].second) << std::endl;
    std::cout << "port: " << std::string(hosts[2].first, hosts[2].second) << std::endl;
    */

    boost::system::error_code             ec;
    boost::asio::ip::tcp::resolver::query query(
        std::string(hosts[1].first, hosts[1].second), std::string(hosts[2].first, hosts[2].second));
    m_endpoints = m_resolver.resolve(query, ec);
    if (ec) {
        if (m_fn_handle_connection_failed) {
            m_fn_handle_connection_failed(shared_from_this(), ec);
        }

        return;
    }

    connect();
}

void TcpTransport::connect() {
    for (auto& v : m_messages) {
        m_allocator.destroy(v.data);
        m_allocator.deallocate(v.data, 1);
    }

    m_messages.clear();

    m_transport_status = transport::EN_READY;

    boost::asio::async_connect(
        m_socket, m_endpoints,
        boost::bind(&TcpTransport::handle_connect, shared_from_this(), boost::asio::placeholders::error));
    m_transport_status = transport::EN_CONNECTING;
}

void TcpTransport::disconnect() {
    boost::asio::post(m_strand, boost::bind(&TcpTransport::handle_close, shared_from_this()));
}

void TcpTransport::accept(const std::string& path) {
    boost::smatch hosts;
    boost::regex  tcp_pattern("tcp://(.*):(\\d+)");
    if (!boost::regex_match(path, hosts, tcp_pattern)) {

        // parse url failed.
        if (m_fn_handle_accept_failed) {
            m_fn_handle_accept_failed(boost::system::errc::make_error_code(boost::system::errc::bad_address));
        }

        return;
    }

    /*
    std::cout << "host: " << std::string(hosts[0].first, hosts[0].second) << std::endl;
    std::cout << "port: " << std::string(hosts[1].first, hosts[1].second) << std::endl;
    std::cout << "port: " << std::string(hosts[2].first, hosts[2].second) << std::endl;
    */

    auto                     port     = std::string(hosts[2].first, hosts[2].second);
    boost::asio::ip::address address  = boost::asio::ip::make_address(std::string(hosts[1].first, hosts[1].second));
    auto                     endpoint = boost::asio::ip::tcp::endpoint(address, atoi(port.c_str()));

    m_acceptor = boost::make_shared<boost::asio::ip::tcp::acceptor>(*m_ioc);

    m_acceptor->open(endpoint.protocol());
    m_acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(1));
    m_acceptor->bind(endpoint);
    m_acceptor->listen();

    do_accept(shared_from_this());
}

void TcpTransport::status(int32_t status) {
    m_transport_status = status;
}

int32_t TcpTransport::status() {
    return m_transport_status;
}

void TcpTransport::connection_made() {
    boost::system::error_code      set_option_err;
    boost::asio::ip::tcp::no_delay no_delay(true);

    m_socket.set_option(no_delay, set_option_err);
    m_transport_status = transport::EN_OK;
    m_local_endpoint   = m_socket.local_endpoint();
    m_remote_endpoint  = m_socket.remote_endpoint();

    // 接收数据
    do_read();
}

void TcpTransport::write(const std::string& data, const transport::on_write_failed& handle_error) {
    std::cout << "pre post thread id: " << boost::this_thread::get_id() << std::endl;
    boost::asio::post(m_strand, [this, data, handle_error]() {
        std::cout << "post, thread id: " << boost::this_thread::get_id() << std::endl;
        // 检查是否有数据正在发送，如果队列中存在数据，则表示有数据正在发送
        bool writing = !m_messages.empty();

        // 通讯状态OK且缓存队列未满，将消息加入缓存队列
        if (m_messages.size() < 10000 && transport::EN_OK == status()) {
            auto s = m_allocator.allocate(1);
            m_allocator.construct(s, data);

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

void TcpTransport::flush() {
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

boost::asio::ip::tcp::endpoint TcpTransport::endpoint(int32_t type) {
    return EN_LOCAL_ENDPOINT == type ? m_local_endpoint : m_remote_endpoint;
}

void TcpTransport::handle_connect(const boost::system::error_code& err) {
    if (!err) {
        boost::system::error_code      set_option_err;
        boost::asio::ip::tcp::no_delay no_delay(true);

        m_socket.set_option(no_delay, set_option_err);

        m_local_endpoint  = m_socket.local_endpoint();
        m_remote_endpoint = m_socket.remote_endpoint();

        if (!set_option_err) {
            m_transport_status = transport::EN_OK;

            if (m_fn_handle_connected) {
                m_fn_handle_connected();
            }

            // 接收数据
            do_read();
        }
    } else {
        m_transport_status = transport::EN_CLOSE;
        std::cout << "handle_connect error, messsage: " << err.message() << std::endl;
        if (m_fn_handle_connection_failed) {
            m_fn_handle_connection_failed(shared_from_this(), err);
        }
    }
}

void TcpTransport::handle_read(const boost::system::error_code& err, size_t length) {
    if (!err) {
        // std::cout << "handle read: " << length << std::endl;

        // 处理接收的数据
        if (m_fn_handle_data_recevied) {
            m_fn_handle_data_recevied(std::string(m_read_data, length));
        }
        m_flow_statistics.increase(FlowStatistics::FlowType::IN, length);
        // 接收数据
        do_read();
    } else {
        std::cout << "handle_read error, message: " << err.message() << std::endl;
        handle_close();

        if (m_fn_handle_connection_lost) {
            m_fn_handle_connection_lost(shared_from_this(), err);
        }
    }
}

void TcpTransport::handle_write(const boost::system::error_code& err, size_t length) {
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

void TcpTransport::handle_accept(const boost::system::error_code& err) {
    std::cout << "this: " << this << ", handle_accept" << std::endl;
    if (!err) {
        do_accept(nullptr);
    } else {
        std::cout << "accept error: " << err.message() << std::endl;
        if (m_fn_handle_accept_failed) {
            m_fn_handle_accept_failed(err);
        }
    }
}

void TcpTransport::handle_close() {
    m_transport_status = transport::EN_CLOSE;
    m_socket.close();
    std::cout << "close_socket" << std::endl;
    if (m_fn_handle_disconnected) {
        m_fn_handle_disconnected();
    }
}

void TcpTransport::do_write() {
    boost::asio::async_write(
        m_socket, boost::asio::buffer(m_messages.front().data->data(), m_messages.front().data->size()),
        boost::asio::transfer_at_least(m_messages.front().data->size()),
        boost::bind(&TcpTransport::handle_write, shared_from_this(), _1, _2));
}

void TcpTransport::do_read() {
    boost::asio::async_read(
        m_socket, boost::asio::buffer(m_read_data, m_read_data_length), boost::asio::transfer_at_least(1),
        boost::bind(&TcpTransport::handle_read, shared_from_this(), _1, _2));
}

void TcpTransport::do_accept(boost::shared_ptr<TcpTransport> transport) {
    std::cout << "this: " << this << ", do_accept(" << transport << ")" << std::endl;
    if (nullptr == transport) {
        transport                            = boost::make_shared<TcpTransport>(m_ioc, m_timeout, m_block_size);
        transport->m_acceptor                = m_acceptor;
        transport->m_fn_handle_accept_failed = m_fn_handle_accept_failed;
    }

    transport->m_acceptor->async_accept(
        transport->m_socket, boost::bind(&TcpTransport::handle_accept, transport, boost::asio::placeholders::error));
}

void TcpTransport::check_deadline() {}
