#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/atomic.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <memory>

class Protocol;

class Transport : public boost::enable_shared_from_this<Transport>
{
public:
    enum {
        EN_READY,
        EN_CONNECTING,
        EN_OK,
        EN_CLOSE
    };

public:
    Transport(boost::asio::io_context& ioc, time_t timeout, size_t block_size);
    ~Transport();

    void set_protocol(boost::shared_ptr<Protocol> protocol);
    boost::shared_ptr<Protocol> protocol();

    /**
     * @brief start 启动通讯管道
     * @param endpoint
     */
    void connect(const boost::asio::ip::tcp::resolver::results_type& endpoints);

    /**
     * @brief Transport::connect
     */
    void connect();

    /**
     * @brief stop 关闭通讯管道
     */
    void disconnect();

    /**
     * @brief status
     * @return
     */
    int32_t status();

    /**
     * @brief write 数据写入接口
     */
    void write(const std::string &data, boost::function<void(const std::string &)> handle_error);

    /**
     * @brief set_on_connected 连接成功数据回调接口
     * @param on_connected 回调接口
     */
    void set_on_connected(boost::function<void()> on_connected);

    /**
     * @brief set_on_disconnected
     * @param on_disconnected
     */
    void set_on_disconnected(boost::function<void()> on_disconnected);

    /**
     * @brief set_data_received 这是消息处理毁掉函数
     * @param on_data_recevied 会掉接口
     */
    void set_on_data_received(boost::function<void(const std::string &data)> on_data_recevied);

    /**
     * @brief set_on_connection_failed
     * @param on_connection_failed
     */
    void set_on_connection_failed(
            boost::function<void(boost::shared_ptr<Transport>, boost::system::error_code)> on_connection_failed);

    /**
     * @brief set_on_connection_lost
     * @param on_connection_lost
     */
    void set_on_connection_lost(
            boost::function<void(boost::shared_ptr<Transport>, boost::system::error_code)> on_connection_lost);
private:
    /**
     * @brief handle_connect 连接处理时间
     * @param err 错误信息
     */
    void handle_connect(const boost::system::error_code& err);

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
     * @brief check_dealine
     */
    void check_dealine();

private:
    boost::asio::io_context::strand m_strand;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::resolver::results_type m_endpoints;

    size_t m_block_size;
    time_t m_timeout;

    char *m_read_data;
    size_t m_read_data_length;

    std::string m_write_data;

    std::allocator<std::string> m_allocator;

    std::list<std::string*> m_messages;

    boost::atomic_int32_t m_transport_status;

    boost::function<void(const std::string &data)> m_on_data_recevied;
    boost::function<void(void)> m_on_connected;
    boost::function<void(void)> m_on_disconnected;
    boost::function<void(boost::shared_ptr<Transport>, boost::system::error_code)> m_on_connection_failed;
    boost::function<void(boost::shared_ptr<Transport>, boost::system::error_code)> m_on_connection_lost;

    boost::shared_ptr<Protocol> m_protocol;
};

#endif // SESSION_H
