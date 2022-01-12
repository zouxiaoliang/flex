#ifndef SSLTRANSPORT_H
#define SSLTRANSPORT_H

#include "BaseTransport.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/container/list.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <memory>

class SslTransport;

class SslTransport : public boost::enable_shared_from_this<SslTransport>, public BaseTransport {
public:
    struct Message {
        std::string*               data;
        transport::on_write_failed on;
    };

    struct FlowStatistics {
        struct ValueType {
            uint64_t count;
            uint64_t bytes;
        };

        enum FlowType {
            IN,
            OUT,
            ERR,

            Max,
        };

        ValueType flow[FlowType::Max];

        void increase(FlowType t, uint64_t var) {
            flow[t].count += 1;
            flow[t].bytes += var;
        }

        bool empty() {
            for (const auto& v : flow) {
                if (0 != v.count || 0 != v.bytes) {
                    return false;
                }
            }
            return true;
        }

        void clear() {
            memset(this, 0x0, sizeof(*this));
        }
    };

    enum { EN_LOCAL_ENDPOINT, EN_REMOTE_ENDPOINT };

public:
    SslTransport(
        boost::shared_ptr<boost::asio::io_context> ioc, time_t timeout, size_t block_size,
        const std::string& certificate_chain_file = "cert.pem", const std::string& password = "",
        const std::string& tmp_dh_file = "");

    ~SslTransport();

    /**
     * @brief connect
     * @param path
     */
    void connect(const std::string& path) override;

    /**
     * @brief connect
     */
    void connect() override;

    /**
     * @brief disconnect
     */
    void disconnect() override;

    /**
     * @brief accept
     * @param path
     */
    void accept(const std::string& path) override;

    /**
     * @brief connection_mode
     */
    void connection_mode() override;

    /**
     * @brief write
     * @param data
     * @param handle_error
     */
    void write(const std::string& data, const transport::on_write_failed& handle_error = {}) override;

    /**
     * @brief flush
     */
    void flush() override;

    /**
     * @brief verify_certificate
     * @param preverified
     * @param ctx
     * @return
     */
    bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx);

    /**
     * @brief endpoint 获取连接地址信息
     * @return
     */
    boost::asio::ip::tcp::endpoint endpoint(int32_t type = EN_LOCAL_ENDPOINT);

protected:
    std::string password() {
        return m_password;
    }
    /**
     * @brief handle_connect 连接处理时间
     * @param err 错误信息
     */
    void handle_connect(const boost::system::error_code& err);

    /**
     * @brief handle_handshake 处理ssl的握手过程
     * @param err
     */
    void handle_handshake(const boost::system::error_code& err);

    /**
     * @brief handle_read 读事件
     * @param err 错误
     * @param length 读到的长度
     */
    void handle_read(const boost::system::error_code& err, size_t length);

    /**
     * @brief handle_write 写事件
     * @param err 错误
     * @param length 已经写的长度
     */
    void handle_write(const boost::system::error_code& err, size_t length);

    /**
     * @brief handle_accept
     * @param err
     */
    void handle_accept(const boost::system::error_code& err);

    /**
     * @brief close_socket 关闭socket连接
     */
    void handle_close();

    /**
     * @brief do_write
     */
    void do_write();

    /**
     * @brief do_read
     */
    void do_read();

    /**
     * @brief do_accept
     * @param transport
     */
    void do_accept(boost::shared_ptr<SslTransport> transport);

private:
    boost::asio::ssl::context                   m_ssl_context;
    typedef boost::asio::ip::tcp::socket        TcpSocket;
    typedef boost::asio::ssl::stream<TcpSocket> SSLSocket;

    boost::shared_ptr<SSLSocket>                 m_ssl_socket;
    boost::asio::ip::tcp::resolver               m_resolver;
    boost::asio::ip::tcp::resolver::results_type m_endpoints;
    boost::asio::ip::tcp::endpoint               m_local_endpoint;
    boost::asio::ip::tcp::endpoint               m_remote_endpoint;

    boost::shared_ptr<boost::asio::io_context>        m_ioc{nullptr};
    boost::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor{nullptr};

    int m_ssl_verify_mode = {boost::asio::ssl::verify_none};

    std::string m_certificate_chain_file;
    std::string m_password;
    std::string m_tmp_dh_file;

    char*  m_read_data{nullptr};
    size_t m_read_data_length{0};

    /**
     * @brief m_allocator
     */
    std::allocator<std::string> m_allocator;

    /**
     * @brief m_messages
     */
    // std::SGIList<std::string*> m_messages;
    // 使用boost::constainer::list 避免sgi版本的list::size函数出现的遍历计算长度的问题
    boost::container::list<Message> m_messages;

    /**
     * @brief m_flow_statistics
     */
    FlowStatistics m_flow_statistics;
};

#endif // SSLTRANSPORT_H
