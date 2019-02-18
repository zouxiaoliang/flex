#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/atomic.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <memory>

#include "KeyVariant.h"

class CBaseProtocol;

class Transport : public boost::enable_shared_from_this<Transport>
{
public:
    /**
     * @brief CallBack 可变类型回调函数
     */
    typedef boost::function<void ()> on_connected;
    typedef boost::function<void ()> on_disconnected;
    typedef boost::function<void(const std::string &)> on_data_recevied;
    typedef boost::function<void(boost::shared_ptr<Transport>, const boost::system::error_code&)> on_connection_lost;
    typedef boost::function<void(boost::shared_ptr<Transport>, const boost::system::error_code&)> on_connection_failed;

    typedef KeyVariant<
        boost::function<void()>,
        boost::function<void(const std::string &)>,
        boost::function<void(boost::shared_ptr<Transport>, const boost::system::error_code&)>
    > TVariantCallBack;
public:
    enum {
        EN_READY,
        EN_CONNECTING,
        EN_OK,
        EN_CLOSE
    };

    enum
    {
        EN_LOCAL_ENDPOINT,
        EN_REMOTE_ENDPOINT
    };

public:
    Transport(boost::asio::io_context& ioc, boost::shared_ptr<boost::asio::ip::tcp::socket> socket, time_t timeout, size_t block_size);
    ~Transport();

    void set_protocol(boost::shared_ptr<CBaseProtocol> protocol);
    boost::shared_ptr<CBaseProtocol> protocol();

    void status(int32_t status){m_transport_status = status;}

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
     * @brief register_callback 回调函数注册
     * @param name
     * @param callback
     * @return
     */
    template<class T>
    bool register_callback(const char *name, T callback)
    {
        if (nullptr == name)
        {
            return false;
        }
        std::cout << "register callback name: " << name << std::endl;
        m_callbacks.set<T>(name, callback);
        return true;
    }

    /**
     * @brief unregister_callback
     * @param name
     */
    void unregister_callback(const char *name)
    {
        if (nullptr == name)
        {
            return;
        }
        m_callbacks.remove(name);
    }

    /**
     * @brief set_data_received 这是消息处理毁掉函数
     * @param on_data_recevied 会掉接口
     */
    void set_on_data_received(boost::function<void(const std::string &data)> on_data_recevied);

    /**
     * @brief endpoint 获取连接地址信息
     * @return
     */
    boost::asio::ip::tcp::endpoint endpoint(int32_t type = EN_LOCAL_ENDPOINT);
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
    boost::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
    boost::asio::ip::tcp::resolver::results_type m_endpoints;

    size_t m_block_size;
    time_t m_timeout;

    char *m_read_data;
    size_t m_read_data_length;

    std::allocator<std::string> m_allocator;

    std::list<std::string*> m_messages;

    boost::atomic_int32_t m_transport_status;

    boost::function<void(const std::string &data)> m_on_data_recevied;

    boost::shared_ptr<CBaseProtocol> m_protocol;

    TVariantCallBack m_callbacks;
};

typedef Transport CBaseTransport;

#endif // SESSION_H
