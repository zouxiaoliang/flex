#include "AutoConnector.h"

AutoReconnector::AutoReconnector(boost::shared_ptr<boost::asio::io_context> ioc, time_t reconnection_cycle) : Connector(ioc), m_reconnection_cycle(reconnection_cycle) {}

AutoReconnector::~AutoReconnector() {}

void AutoReconnector::__build_protocol(
    boost::shared_ptr<BaseTransport> transport,
    boost::shared_ptr<BaseProtocol>  protocol) {
    if (m_fn_connection_lost) {
        transport->bind_handle_connection_lost(m_fn_connection_lost);
    } else {
        transport->bind_handle_connection_lost(boost::bind(&AutoReconnector::connection_lost, this, _1, _2));
    }

    if (m_fn_connection_failed) {
        transport->bind_handle_connection_failed(m_fn_connection_failed);
    } else {
        transport->bind_handle_connection_failed(boost::bind(&AutoReconnector::connection_failed, this, _1, _2));
    }
}

void AutoReconnector::connection_lost(
    boost::shared_ptr<BaseTransport> connector,
    const boost::system::error_code& err) {
    if (err) {
        boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(m_reconnection_cycle));
        std::cout << "connection lost, retry connect to server" << std::endl;
        connector->connect();
    }
}

void AutoReconnector::connection_failed(
    boost::shared_ptr<BaseTransport> connector,
    const boost::system::error_code& err) {
    if (err) {
        boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(m_reconnection_cycle));
        std::cout << "connection failed, retry connect to server, what: " << err.message() << std::endl;
        connector->connect();
    }
}
