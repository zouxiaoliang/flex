/**
 * @author zouxiaoliang
 * @date 2021/11/10
 */
#ifndef FLEX_NETLINKTRANSPORT_H
#define FLEX_NETLINKTRANSPORT_H

#include <boost/enable_shared_from_this.hpp>
#include <list>
#include <memory>

#include "BaseTransport.h"
#include "boost/asio/netlink.hpp"

namespace flex {
/**
 * @brief The NetlinkTransport class
 * @ref https://stackoverflow.com/questions/13255447/af-netlink-netlink-sockets-using-boostasio
 */
class NetlinkTransport : public boost::enable_shared_from_this<NetlinkTransport>, public BaseTransport {
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
    NetlinkTransport(
        boost::shared_ptr<boost::asio::io_context> ioc, time_t timeout, size_t block_size, int32_t pid = getpid(),
        int32_t group = 0, int proto = 0);
    virtual ~NetlinkTransport();

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
    void accept(const std::string& path, boost::function<void(boost::shared_ptr<BaseTransport>)> on_accept) override;

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

protected:
    /**
     * @brief handle_connect
     * @param err
     */
    void handle_connect(const boost::system::error_code& err);

    /**
     * @brief handle_read
     * @param err
     * @param length
     */
    void handle_read(const boost::system::error_code& err, size_t length);

    /**
     * @brief handle_write
     * @param err
     * @param length
     */
    void handle_write(const boost::system::error_code& err, size_t length);

    /**
     * @brief handle_close
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

private:
    void nlmsg_pack(std::string*& buffer, const std::string& data);

    bool nlmsg_unpack(const char* buffer, size_t length, std::string& message);

protected:
    boost::asio::netlink::socket    m_socket;
    boost::asio::netlink::endpoints m_endpoints;

    boost::shared_ptr<boost::asio::io_context> m_ioc{nullptr};

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
    std::list<Message> m_messages;

    /**
     * @brief m_flow_statistics
     */
    FlowStatistics m_flow_statistics;

    int32_t m_pid{-1};
    int32_t m_group{0};
    int32_t m_proto{0};
};

} // namespace flex
#endif // NETLINKTRANSPORT_H
