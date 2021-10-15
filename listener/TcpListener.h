#ifndef TCPLISTENER_H
#define TCPLISTENER_H


#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class Connector;
class TcpTransport;
class BaseProtocol;

class TcpListener : public boost::enable_shared_from_this<TcpListener>
{
public:

    TcpListener(){}

    virtual ~TcpListener(){}

public:

    /**
     * @brief listen 启动监听，并且异步等待连接请求
     * @param ioc
     * @param endpoint 监听地址
     * @param factory protocol工厂对象
     */
    template <class ProtocolType>
    void listen(
        boost::shared_ptr<boost::asio::io_context> ioc,
        const boost::asio::ip::tcp::endpoint&      endpoint,
        boost::shared_ptr<Connector>               factory);

protected:
    template <class ProtocolType>
    void do_accept(
        boost::shared_ptr<boost::asio::io_context>        ioc,
        boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor,
        boost::shared_ptr<Connector>                      factory);

    /**
     * @brief handle_accept accept回调处理函数
     * @param ioc
     * @param acceptor
     * @param socket
     * @param factory
     * @param err
     */
    template <class ProtocolType>
    void handle_accept(
        boost::shared_ptr<boost::asio::io_context>            ioc,
        boost::shared_ptr<boost::asio::ip::tcp::acceptor>     acceptor,
        const boost::shared_ptr<boost::asio::ip::tcp::socket> socket,
        boost::shared_ptr<Connector>                          factory,
        const boost::system::error_code&                      err);

    virtual void connection_made(boost::shared_ptr<BaseProtocol> session);
};

#include "TcpListener.inl"
#endif // TCPLISTENER_H
