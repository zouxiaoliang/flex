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

class TcpTransport : public boost::enable_shared_from_this<TcpTransport>,
                     public BaseTransport
{
public:
    struct Message
    {
        std::string *data;
        transport::on_write_failed on;
    };

    struct FlowStatistics
    {
        struct ValueType
        {
            uint64_t count;
            uint64_t bytes;
        };

        enum FlowType
        {
            IN,
            OUT,
            ERR,

            Max,
        };

        ValueType flow[FlowType::Max];

        void increase(FlowType t, uint64_t var)
        {
            flow[t].count += 1;
            flow[t].bytes += var;
        }

        bool empty()
        {
            for (const auto &v : flow)
            {
                if (0 != v.count || 0 != v.bytes)
                {
                    return false;
                }
            }
            return true;
        }

        void clear()
        {
            memset(this, 0x0, sizeof(*this));
        }
    };

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
     * @param m_resolver
     */
    void connect(const std::string& path) override;

    /**
     * @brief Transport::connect
     */
    void connect() override;

    /**
     * @brief stop 关闭通讯管道
     */
    void disconnect() override;

    /**
     * @brief status 当前transport的状态
     * @return
     */
    int32_t status() override;

    /**
     * @brief connection_made
     */
    void connection_made();

    /**
     * @brief write 数据写入接口，针对特定的消息如果发送失败需要特殊处理，则使用该接口
     * @brief handle_error 数据写入失败的处理函数
     */
    void write(const std::string &data, const transport::on_write_failed &handle_error = {}) override;

    /**
     * @brief flush
     */
    void flush() override;

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
    boost::asio::ip::tcp::resolver m_resolver;
    boost::asio::ip::tcp::resolver::results_type m_endpoints;
    boost::asio::ip::tcp::endpoint m_local_endpoint;
    boost::asio::ip::tcp::endpoint m_remote_endpoint;

    char *m_read_data;
    size_t m_read_data_length;

    /**
     * @brief m_allocator
     */
    std::allocator<std::string> m_allocator;

    /**
     * @brief m_messages
     */
    // std::SGIList<std::string*> m_messages;
    std::SGIList<Message> m_messages;

    /**
     * @brief m_flow_statistics
     */
    FlowStatistics m_flow_statistics;
};

#endif // TCPTRANSPORT_H
