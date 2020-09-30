#include "protocol/BaseProtocol.h"
#include "transport/TcpTransport.h"

#include <boost/make_shared.hpp>
#include <boost/chrono.hpp>
#include <boost/chrono/duration.hpp>
#include <boost/bind.hpp>

#include <iostream>

BaseProtocol::BaseProtocol(boost::shared_ptr<boost::asio::io_context> ioc, boost::shared_ptr<TcpTransport> transport) :
    m_ioc(ioc),
    m_transport(transport)
{
    if (m_transport)
    {
        // register event handler
        m_transport->register_callback<boost::function<void()>>("on_connected", boost::bind(&BaseProtocol::on_connected, this));
        m_transport->register_callback<boost::function<void()>>("on_disconnected", boost::bind(&BaseProtocol::on_disconnected, this));
        m_transport->register_callback<boost::function<void(const std::string &)>>("on_raw_data_received", boost::bind(
                &BaseProtocol::on_raw_data_received, this, _1));
        m_transport->register_callback<boost::function<void()>>("on_write_completed", boost::bind(&BaseProtocol::on_write_completed, this));

        m_transport->set_on_read(boost::bind(&BaseProtocol::on_raw_data_received, this, _1));
    }
}

BaseProtocol::~BaseProtocol() = default;

void BaseProtocol::write(const std::string &message)
{
    if (m_transport)
    {
        m_transport->write(message, boost::bind(&BaseProtocol::on_write_error, this, _1));
    }
}

void BaseProtocol::close()
{
    if (m_transport)
    {
        m_transport->disconnect();
    }
}

int BaseProtocol::transport_status()
{
    if (m_transport)
    {
        return m_transport->status();
    }
    return transport::EN_CLOSE;
}

void BaseProtocol::on_connected()
{

}

void BaseProtocol::on_disconnected()
{

}

void BaseProtocol::on_write_error(const std::string &data)
{

}

void BaseProtocol::on_write_completed()
{

}

void BaseProtocol::on_message_received(const std::string &message)
{

}

void BaseProtocol::on_raw_data_received(const std::string &data)
{

}
