#ifndef BASEACCEPTOR_H
#define BASEACCEPTOR_H


#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class CBaseFactory;
class Transport;
class CBaseProtocol;

/**
 * @brief The CAcceptor class 启动监听，并为终端连接请求创建protocol对象
 */
class CAcceptor : public boost::enable_shared_from_this<CAcceptor>
{
public:

    CAcceptor(boost::asio::io_context& ioc, boost::shared_ptr<CBaseFactory> factory);

    virtual ~CAcceptor();
public:

    /**
     * @brief listen 启动监听，并且异步等待连接请求
     * @param endpoint
     */
    void listen(const boost::asio::ip::tcp::endpoint &endpoint);
private:

    /**
     * @brief do_accept
     */
    void do_accept();

    /**
     * @brief handle_accept accept回调处理函数
     * @param session
     * @param err
     */
    void handle_accept(const boost::shared_ptr<boost::asio::ip::tcp::socket> session, const boost::system::error_code& err);
protected:

    /**
     * @brief connection_made 接收到连接请求后自定义处理函数
     * @param session
     * @param err
     */
    virtual void connection_made(const boost::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code &err);
protected:

    boost::asio::io_context& m_ioc;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::shared_ptr<CBaseFactory> m_factory;
};

class CAcceptorV2 : public boost::enable_shared_from_this<CAcceptorV2>
{
public:

    CAcceptorV2(){}

    virtual ~CAcceptorV2(){}

public:

    /**
     * @brief listen 启动监听，并且异步等待连接请求
     * @param ioc
     * @param endpoint 监听地址
     * @param factory  protocol工厂对象
     */
    template <class ProtocolType>
    void listen(boost::shared_ptr<boost::asio::io_context> ioc, const boost::asio::ip::tcp::endpoint& endpoint, boost::shared_ptr<CBaseFactory> factory);
protected:

    template <class ProtocolType>
    void do_accept(boost::shared_ptr<boost::asio::io_context> ioc,
                   boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor,
                   boost::shared_ptr<CBaseFactory> factory);


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
            boost::shared_ptr<boost::asio::io_context> ioc,
            boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor,
            const boost::shared_ptr<boost::asio::ip::tcp::socket> socket,
            boost::shared_ptr<CBaseFactory> factory,
            const boost::system::error_code &err);

    virtual void connection_made(boost::shared_ptr<CBaseProtocol> session);
};

#include "BaseAcceptor.inl"
#endif // BASEACCEPTOR_H
