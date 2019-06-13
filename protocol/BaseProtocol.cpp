#include "BaseProtocol.h"
#include "transport/Transport.h"

#include <boost/make_shared.hpp>
#include <boost/chrono.hpp>
#include <boost/chrono/duration.hpp>
#include <boost/bind.hpp>

#include <arpa/inet.h>

#include <iostream>

CBaseProtocol::CBaseProtocol(boost::asio::io_context &ioc, boost::shared_ptr<CBaseTransport> transport) :
    m_ioc(ioc),
    m_transport(transport)
{
    if (m_transport)
    {
        m_transport->register_callback<boost::function<void()>>("on_connected", boost::bind(&CBaseProtocol::on_connected, this));
        m_transport->register_callback<boost::function<void()>>("on_disconnected", boost::bind(&CBaseProtocol::on_disconnected, this));
        m_transport->register_callback<boost::function<void(const std::string &)>>("on_data_received", boost::bind(&CBaseProtocol::on_data_received, this, _1));
        m_transport->set_on_data_received(boost::bind(&CBaseProtocol::on_data_received, this, _1));
    }
}

CBaseProtocol::~CBaseProtocol()
{

}

void CBaseProtocol::write(const std::string &message)
{
    if (m_transport)
    {
        m_transport->write(message, boost::bind(&CBaseProtocol::on_write_error, this, _1));
    }
}

void CBaseProtocol::close()
{
    if (m_transport)
    {
        m_transport->disconnect();
    }
}

int CBaseProtocol::transport_status()
{
    if (m_transport)
    {
        return m_transport->status();
    }
    return CBaseTransport::EN_CLOSE;
}

void CBaseProtocol::on_connected()
{

}

void CBaseProtocol::on_disconnected()
{

}

void CBaseProtocol::on_write_error(const std::string &data)
{

}

void CBaseProtocol::message_received(const std::string &message)
{

}

void CBaseProtocol::on_data_received(const std::string &data)
{

}
