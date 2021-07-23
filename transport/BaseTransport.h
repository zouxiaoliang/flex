#ifndef BASETRANSPORT_H
#define BASETRANSPORT_H

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/atomic.hpp>
#include <string>

#include "utils/KeyVariant.h"

class BaseTransport;
class BaseProtocol;

namespace transport {

/**
 * @brief CallBack 可变类型回调函数
 */
typedef boost::function<void ()> on_connected;
typedef boost::function<void ()> on_disconnected;
typedef boost::function<void(const std::string &)> on_data_recevied;
typedef boost::function<void(boost::shared_ptr<BaseTransport>, const boost::system::error_code&)> on_connection_lost;
typedef boost::function<void(boost::shared_ptr<BaseTransport>, const boost::system::error_code&)> on_connection_failed;
typedef boost::function<void ()> on_write_completed;

/**
 * transport 状态
 */
enum ETransportStatus
{
    EN_READY,
    EN_CONNECTING,
    EN_OK,
    EN_CLOSE
};

enum EEventCallback
{
    // EVENT TRICK
    EN_ON_CONNCETED,        // connected to server
    EN_ON_DISCONNECTED,     // disconnected to server
    EN_WRITE_COMPLETED,  // pre-send buffer is empty.
    EN_DATA_RECEIVED,       // recv message from server.  

    // ERROR OR FAILED
    EN_CONNECTION_LOST,     // network aviable, the session closed or other unknown error.
    EN_CONNECTION_FAILED,   // can't connect to the server.
};
}

class BaseTransport
{
public:
    BaseTransport(boost::shared_ptr<boost::asio::io_context> ioc,
                  time_t timeout,
                  size_t block_size) :
        m_strand(*ioc), m_timeout(timeout), m_block_size(block_size), m_transport_status(transport::EN_CLOSE) {}

    virtual ~BaseTransport() = default;

    /**
     * @brief set_protocol 设置应用层处理协议
     * @param protocol 协议处理独享
     */
    void set_protocol(boost::shared_ptr<BaseProtocol> protocol) { m_protocol = protocol; }

    /**
     * @brief protocol 获取协议对象
     * @return
     */
    boost::shared_ptr<BaseProtocol> protocol() { return m_protocol; }

    /**
     * @brief connect 连接服务端
     * @param path 服务端信息
     */
    virtual void connect(const std::string &path) {}

    /**
     * @brief connect 连接
     */
    virtual void connect() {}

    /**
     * @brief disconnect 断开连接
     */
    virtual void disconnect() {}

    /**
     * @brief status 当前状态
     * @ref ETransportStatus
     * @return
     */
    virtual int32_t status() { return m_transport_status; }

    /**
     * @brief connection_mode 连接建立完成
     */
    virtual void connection_mode() {}

    /**
     * @brief write 发送消息
     * @param data 消息
     * @param handle_error 发送失败处理回调
     */
    void write(const std::string &data, boost::function<void(const std::string&)> handle_error = nullptr) {}

    /**
     * @brief flush 清空发送缓冲
     */
    virtual void flush() {}

    /**
     * @brief set_on_read 设置read响应处理
     * @param on_read 处理借口
     */
    // clang-format off
    [[deprecated("instead use bind_handle_data_recevied")]]
    // clang-format on
    void set_on_read(boost::function<void(const std::string &data)> on_read) { m_fn_handle_data_recevied = on_read; }

    void bind_handle_connected(transport::on_connected on) { m_fn_handle_connected = on; }
    void bind_handle_disconnected(transport::on_disconnected on) { m_fn_handle_disconnected = on; }
    void bind_handle_data_recevied(transport::on_data_recevied on) { m_fn_handle_data_recevied = on; }
    void bind_handle_connection_lost(transport::on_connection_lost on) { m_fn_handle_connection_lost = on; }
    void bind_handle_connection_failed(transport::on_connection_failed on) { m_fn_handle_connection_failed = on; }
    void bind_handle_write_completed(transport::on_write_completed on) { m_fn_handle_write_completed = on; }

    /**
     * @brief reset_callback 重置回调函数接口
     * @param callback_id
     */
    void reset_callback(transport::EEventCallback callback_id)
    {
        switch (callback_id)
        {
        case transport::EEventCallback::EN_ON_CONNCETED:
            m_fn_handle_connected.clear();
            break;
        case transport::EEventCallback::EN_ON_DISCONNECTED:
            m_fn_handle_disconnected.clear();
            break;
        case transport::EEventCallback::EN_WRITE_COMPLETED:
            m_fn_handle_write_completed.clear();
            break;
        case transport::EEventCallback::EN_DATA_RECEIVED:
            m_fn_handle_data_recevied.clear();
            break;
        case transport::EEventCallback::EN_CONNECTION_LOST:
            m_fn_handle_connection_lost.clear();
            break;
        case transport::EEventCallback::EN_CONNECTION_FAILED:
            m_fn_handle_connection_failed.clear();
            break;
        default:
            break;
        }
    }

protected:
    /// @brief 上下文
    boost::asio::io_context::strand m_strand;

    /// @brief 超市时间
    time_t m_timeout;
    /// @brief 数据块大小
    size_t m_block_size;
    /// @brief 当前状态
    boost::atomic_int32_t m_transport_status;

    /// @brief 处理协议的引用
    boost::shared_ptr<BaseProtocol> m_protocol;

    transport::on_connected m_fn_handle_connected;
    transport::on_disconnected m_fn_handle_disconnected;
    transport::on_data_recevied m_fn_handle_data_recevied;
    transport::on_connection_lost m_fn_handle_connection_lost;
    transport::on_connection_failed m_fn_handle_connection_failed;
    transport::on_write_completed m_fn_handle_write_completed;
};


#endif //BASETRANSPORT_H
