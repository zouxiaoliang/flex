#ifndef TCPTRANSPORT_H
#define TCPTRANSPORT_H

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/atomic.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <memory>

#include "BaseTransport.h"

#include "utils/KeyVariant.h"
#include "utils/sgi_plus.h"

class TcpTransport;

namespace tcp {

/**
 * @brief CallBack 可变类型回调函数
 */
typedef boost::function<void ()> on_connected;
typedef boost::function<void ()> on_disconnected;
typedef boost::function<void(const std::string &)> on_data_recevied;
typedef boost::function<void(boost::shared_ptr<TcpTransport>, const boost::system::error_code&)> on_connection_lost;
typedef boost::function<void(boost::shared_ptr<TcpTransport>, const boost::system::error_code&)> on_connection_failed;
typedef boost::function<void ()> on_write_completed;

typedef KeyVariant<
    boost::function<void()>,
    boost::function<void (bool)>,
    boost::function<void(const std::string &)>,
    boost::function<void(boost::shared_ptr<TcpTransport>, const boost::system::error_code&)>
> TOnEvent;

}

class TcpTransport : public boost::enable_shared_from_this<TcpTransport> ,
        public BaseTransport<boost::asio::ip::tcp::resolver::results_type, tcp::TOnEvent>
{
public:
    enum
    {
        EN_LOCAL_ENDPOINT,
        EN_REMOTE_ENDPOINT
    };

public:
    TcpTransport(
            boost::shared_ptr<boost::asio::io_context> ioc,
            boost::shared_ptr<boost::asio::ip::tcp::socket> socket,
            time_t timeout,
            size_t block_size
            );
    virtual ~TcpTransport();

    /**
     * @brief status 设置当前transport的状态
     * @param status
     */
    void status(int32_t status);

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
     * @brief connection_made
     */
    void connection_made();

    /**
     * @brief write 数据写入接口
     */
    void write(const std::string &data, boost::function<void(const std::string &)> handle_error);

    /**
     * @brief flush
     */
    void flush();

    /**
     * @brief endpoint 获取连接地址信息
     * @return
     */
    boost::asio::ip::tcp::endpoint endpoint(int32_t type = EN_LOCAL_ENDPOINT);
protected:

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
     * @brief check_deadline
     */
    void check_deadline();

protected:
    boost::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
    boost::asio::ip::tcp::resolver::results_type m_endpoints;
    boost::asio::ip::tcp::endpoint m_local_endpoint;
    boost::asio::ip::tcp::endpoint m_remote_endpoint;

    char *m_read_data;
    size_t m_read_data_length;

    std::allocator<std::string> m_allocator;

    std::SGIList<std::string*> m_messages;
};

#endif // TCPTRANSPORT_H
