#include "BaseTransport.h"

const std::string& BaseTransport::get_local_address() const {
    return m_local_address;
}

void BaseTransport::set_local_address(const std::string& new_local_address) {
    m_local_address = new_local_address;
}

const std::string& BaseTransport::get_remote_address() const {
    return m_remote_address;
}

void BaseTransport::set_remote_address(const std::string& new_remote_address) {
    m_remote_address = new_remote_address;
}
