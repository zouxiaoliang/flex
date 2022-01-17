#include "SslTransport.h"

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>

#include <iostream>

SslTransport::SslTransport(
    boost::shared_ptr<boost::asio::io_context> ioc, time_t timeout, size_t block_size, boost::function<bool(const std::string&)> handshake_check, const std::string& certificate_chain_file,
    const std::string& password, const std::string& tmp_dh_file)
    : BaseTransport(ioc, timeout, block_size), m_ssl_context(boost::asio::ssl::context::sslv23), m_ssl_socket(nullptr), m_resolver(*ioc), m_ioc(ioc), m_fn_handshake_check(handshake_check),
      m_certificate_chain_file(certificate_chain_file), m_password(password), m_tmp_dh_file(tmp_dh_file), m_read_data(new char[block_size]), m_read_data_length(block_size) {}

SslTransport::~SslTransport() {
    if (nullptr != m_read_data) {
        delete[] m_read_data;
    }
}

void SslTransport::connect(const std::string& path) {

    boost::smatch hosts;
    boost::regex  ssl_pattern("ssl://(.*):(\\d+)");
    if (!boost::regex_match(path, hosts, ssl_pattern)) {
        if (m_fn_handle_connection_failed) {
            m_fn_handle_connection_failed(
                shared_from_this(), boost::system::errc::make_error_code(boost::system::errc::bad_address));
        }

        return;
    }

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

    if (boost::filesystem::exists(m_certificate_chain_file, ec)) {
        m_ssl_context.load_verify_file(m_certificate_chain_file);
        m_ssl_verify_mode = boost::asio::ssl::verify_peer;
    }

    connect();
}

void SslTransport::connect() {
    m_ssl_socket = boost::make_shared<SSLSocket>(*m_ioc, m_ssl_context);
    m_ssl_socket->set_verify_mode(m_ssl_verify_mode);
    m_ssl_socket->set_verify_callback(boost::bind(&SslTransport::verify_certificate, this, boost::placeholders::_1, boost::placeholders::_2));

    boost::asio::async_connect(m_ssl_socket->lowest_layer(), m_endpoints, boost::bind(&SslTransport::handle_connect, shared_from_this(), boost::asio::placeholders::error));

    m_transport_status = transport::EN_CONNECTING;
}

void SslTransport::disconnect() {
    boost::asio::post(m_strand, boost::bind(&SslTransport::handle_close, shared_from_this()));
}

void SslTransport::accept(const std::string& path) {

    boost::smatch hosts;
    boost::regex  tcp_pattern("ssl://(.*):(\\d+)");
    if (!boost::regex_match(path, hosts, tcp_pattern)) {

        // parse url failed.
        if (m_fn_handle_accept_failed) {
            m_fn_handle_accept_failed(boost::system::errc::make_error_code(boost::system::errc::bad_address));
        }

        return;
    }

    auto                     port     = std::string(hosts[2].first, hosts[2].second);
    boost::asio::ip::address address  = boost::asio::ip::make_address(std::string(hosts[1].first, hosts[1].second));
    auto                     endpoint = boost::asio::ip::tcp::endpoint(address, atoi(port.c_str()));

    m_acceptor = boost::make_shared<boost::asio::ip::tcp::acceptor>(*m_ioc);

    m_acceptor->open(endpoint.protocol());
    m_acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(1));
    m_acceptor->bind(endpoint);
    m_acceptor->listen();

    // set options default_workarounds | no_sslv2 | single_dh_use
    namespace ssl = boost::asio::ssl;
    boost::system::error_code err;
#define USING_DH 1
#define USING_SSLV23 1
    do {
        m_ssl_context.set_options(
            ssl::context::default_workarounds
#if USING_SSLV23
            | ssl::context::sslv23
#else
            | ssl::context::no_sslv2
#endif
#if USING_DH
            | ssl::context::single_dh_use
#endif
        );
        // set password
        m_ssl_context.set_password_callback(boost::bind(&SslTransport::password, this));
        // set certificate chain file
        m_ssl_context.use_certificate_chain_file(m_certificate_chain_file, err);
        if (err) {
            break;
        }
        // set private key file
        m_ssl_context.use_private_key_file(m_certificate_chain_file, ssl::context::pem, err);
        if (err) {
            break;
        }
#if USING_DH
        // set tmp dh file
        m_ssl_context.use_tmp_dh_file(m_tmp_dh_file, err);
#endif
        if (err) {
            break;
        }
    } while (false);

    if (err) {
        std::cout << "accept failed, error: " << err.message() << std::endl;
        if (m_fn_handle_accept_failed) {
            m_fn_handle_accept_failed(err);
        }
        return;
    }

    m_ssl_socket = boost::make_shared<SSLSocket>(*m_ioc, m_ssl_context);

    // start accept
    do_accept(shared_from_this());
}

void SslTransport::connection_mode() {
    boost::system::error_code      set_option_err;
    boost::asio::ip::tcp::no_delay no_delay(true);

    // set socket option
    m_ssl_socket->lowest_layer().set_option(no_delay, set_option_err);

    // update transoport status and record loacl/remote address
    m_transport_status = transport::EN_OK;
    m_local_endpoint   = m_ssl_socket->lowest_layer().local_endpoint();
    m_remote_endpoint  = m_ssl_socket->lowest_layer().remote_endpoint();

    std::cout << "(" << m_local_endpoint.address().to_string() << ":" << m_local_endpoint.port() << ") -> ("
              << m_remote_endpoint.address().to_string() << ":" << m_remote_endpoint.port() << ")" << std::endl;
}

void SslTransport::write(const std::string& data, const transport::on_write_failed& handle_error) {
    // std::cout << "pre post thread id: " << boost::this_thread::get_id() << std::endl;
    boost::asio::post(m_strand, [this, data, handle_error]() {
        // std::cout << "post, thread id: " << boost::this_thread::get_id() << std::endl;
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

void SslTransport::flush() {
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

bool SslTransport::verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx) {
    char subject_name[256] = {};

    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);

    std::cout << "Verifying " << subject_name << ", preverified is " << (preverified ? "True" : "False") << std::endl;
    if (m_fn_handshake_check) {
        return m_fn_handshake_check(subject_name);
    }
    return preverified;
}

boost::asio::ip::tcp::endpoint SslTransport::endpoint(int32_t type) {
    return EN_LOCAL_ENDPOINT == type ? m_local_endpoint : m_remote_endpoint;
}

void SslTransport::handle_connect(const boost::system::error_code& err) {
    if (!err) {
        // update some infomations and flsga, and notify the caller connnection is ok.
        connection_mode();

        // ssl handshake
        m_ssl_socket->async_handshake(
            boost::asio::ssl::stream_base::client,
            boost::bind(&SslTransport::handle_handshake, shared_from_this(), boost::asio::placeholders::error));

    } else {
        m_transport_status = transport::EN_CLOSE;
        std::cout << "handle_connect error, messsage: " << err.message() << std::endl;
        if (m_fn_handle_connection_failed) {
            m_fn_handle_connection_failed(shared_from_this(), err);
        }
    }
}

void SslTransport::handle_handshake(const boost::system::error_code& err) {
    if (!err) {
        // now, hand shake is ok, you can recv data from server.
        // notify the caller, connection is ok
        if (m_fn_handle_connected) {
            m_fn_handle_connected();
        }
        do_read();
    } else {
        m_transport_status = transport::EN_CLOSE;
        disconnect();

        if (m_fn_handle_connection_failed) {
            m_fn_handle_connection_failed(shared_from_this(), err);
        }
        std::cout << "ssl handshake failed, error: " << err.message() << std::endl;
    }
}

void SslTransport::handle_read(const boost::system::error_code& err, size_t length) {
    if (!err) {
        std::cout << "handle read: " << length << std::endl;

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

void SslTransport::handle_write(const boost::system::error_code& err, size_t length) {
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

void SslTransport::handle_accept(const boost::system::error_code& err) {
    if (!err) {
        // start session, hand shake
        m_ssl_socket->async_handshake(
            boost::asio::ssl::stream_base::server,
            boost::bind(&SslTransport::handle_handshake, shared_from_this(), boost::asio::placeholders::error));

        // next accept
        do_accept(nullptr);
    } else {
        std::cout << "accept error: " << err.message() << std::endl;
        if (m_fn_handle_accept_failed) {
            m_fn_handle_accept_failed(err);
        }
    }
}

void SslTransport::handle_close() {
    m_transport_status = transport::EN_CLOSE;
    m_ssl_socket->lowest_layer().close();
    std::cout << "close_socket" << std::endl;
    if (m_fn_handle_disconnected) {
        m_fn_handle_disconnected();
    }
}

void SslTransport::do_write() {
    boost::asio::async_write(
        *m_ssl_socket, boost::asio::buffer(m_messages.front().data->data(), m_messages.front().data->size()),
        boost::asio::transfer_at_least(m_messages.front().data->size()),
        boost::bind(&SslTransport::handle_write, shared_from_this(), _1, _2));
}

void SslTransport::do_read() {
    boost::asio::async_read(
        *m_ssl_socket, boost::asio::buffer(m_read_data, m_read_data_length), boost::asio::transfer_at_least(1),
        boost::bind(&SslTransport::handle_read, shared_from_this(), _1, _2));
}

void SslTransport::do_accept(boost::shared_ptr<SslTransport> transport) {
    // acceptor aysnc accept, using a new ssl_socket;
    if (nullptr == transport) {
        transport                            = boost::make_shared<SslTransport>(m_ioc, m_timeout, m_block_size, m_fn_handshake_check, m_certificate_chain_file, m_password, m_tmp_dh_file);
        transport->m_acceptor                = m_acceptor;
        transport->m_fn_handle_accept_failed = m_fn_handle_accept_failed;

        namespace ssl = boost::asio::ssl;
        boost::system::error_code err;
        do {
            transport->m_ssl_context.set_options(
                ssl::context::default_workarounds
#if USING_SSLV23
                | ssl::context::sslv23
#else
                | ssl::context::no_sslv2
#endif
#if USING_DH
                | ssl::context::single_dh_use
#endif
            );
            // set password
            transport->m_ssl_context.set_password_callback(boost::bind(&SslTransport::password, this));
            // set certificate chain file
            transport->m_ssl_context.use_certificate_chain_file(m_certificate_chain_file, err);
            if (err) {
                break;
            }
            // set private key file
            transport->m_ssl_context.use_private_key_file(m_certificate_chain_file, ssl::context::pem, err);
            if (err) {
                break;
            }
#if USING_DH
            // set tmp dh file
            transport->m_ssl_context.use_tmp_dh_file(m_tmp_dh_file, err);
#endif
            if (err) {
                break;
            }
        } while (false);

        if (err) {
            std::cout << "accept failed, error: " << err.message() << std::endl;
            if (transport->m_fn_handle_accept_failed) {
                transport->m_fn_handle_accept_failed(err);
            }
            return;
        }
        transport->m_ssl_socket = boost::make_shared<SSLSocket>(*m_ioc, m_ssl_context);
    }

    transport->m_acceptor->async_accept(
        transport->m_ssl_socket->lowest_layer(),
        boost::bind(&SslTransport::handle_accept, transport, boost::asio::placeholders::error));
}
