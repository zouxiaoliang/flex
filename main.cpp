#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include <inttypes.h>
#include <iostream>
#include <thread>

#include "factory/AutoConnector.h"
#include "listener/Listener.h"
#include "protocol/GenericProtocol.h"
#include "transport/SslTransport.h"
#include "transport/TcpTransport.h"

using namespace std;
#ifndef PRIu64
#define __PRI_64_LENGTH_MODIFIER__ "ll"
#define PRIu64 __PRI_64_LENGTH_MODIFIER__ "u"
#endif

enum class ServerType {
    Tcp,
    SSL,
};

static const auto g_server_type = ServerType::SSL;

const char* fmt = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
                  "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%" PRIu64;

namespace fn {
void sleep(long sec) {
    boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(sec));
}

void msleep(long millisec) {
    boost::thread::sleep(boost::get_system_time() + boost::posix_time::millisec(millisec));
}
} // namespace fn

boost::system_time future(long sec) {
    return boost::get_system_time() + boost::posix_time::seconds(sec);
}

void start_client(const std::string& url, uint64_t count, uint64_t client_count) {
    auto ioc = boost::make_shared<boost::asio::io_context>();

    auto reconnect_factory = boost::make_shared<AutoReconnector>(ioc);

    // clients
    std::vector<boost::shared_ptr<BaseProtocol>> clients;
    for (int var = 0; var < client_count; ++var) {
        if (g_server_type == ServerType::Tcp) {
            auto protocol = reconnect_factory->connect<GenericProtocol, TcpTransport>(url, 10, 1024);
            if (protocol) {
                clients.push_back(protocol);
            }
        } else {
            auto protocol = reconnect_factory->connect<GenericProtocol, SslTransport>(url, 10, 1024, default_handshake_check, "public.pem");
            if (protocol) {
                clients.push_back(protocol);
            }
        }
    }

    char buffer[2048] = {};

    std::thread thr_ioc([&ioc]() { ioc->run(); });
    std::thread thr_writer([&]() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
        for (uint64_t var = 0; true;) {
            for (const auto& p : clients) {
                if (transport::EN_OK != p->transport_status()) {
                    fn::sleep(1);
                    continue;
                }

                std::sprintf(buffer, fmt, var);
                p->write(buffer);
            }
            if ((var % count) == 0) {
                fn::sleep(1);
                std::cout << "send count: " << var << " * " << client_count << std::endl;
            }
            ++var;
        }
#pragma clang diagnostic pop
    });

    // wait the thread stopped
    thr_writer.join();
    thr_ioc.join();

    // close clients
    for (auto p : clients) {
        p->close();
    }
}

void start_server() {

    auto ioc = boost::make_shared<boost::asio::io_context>();
    if (g_server_type == ServerType::Tcp) {
        // auto transport = boost::make_shared<TcpTransport>(ioc, 100, 1024);
        // transport->accept("tcp://127.0.0.1:8888");

        Listener listener(ioc);
        listener.listen<GenericProtocol, TcpTransport>("tcp://127.0.0.1:8888", 100, 1024);
    } else {
        // auto transport = boost::make_shared<SslTransport>(ioc, 100, 1024, "private.pem", "helloworld",
        // "dh2048.pem"); transport->accept("ssl://127.0.0.1:8889");

        Listener listener(ioc);
        listener.listen<GenericProtocol, SslTransport>("ssl://127.0.0.1:8889", 100, 1024, default_handshake_check, "private.pem", "helloworld", "dh2048.pem");
    }

    ioc->run();
}

class Helloworld {
public:
    void helloworld(int i1, int i2, int i3, int i4, int i5, int i6) {
        std::cout << i1 + i2 + i3 + i4 << std::endl;
    }
};

int main(int argc, char* argv[]) {
    auto fn = boost::bind(start_client, _1, _2, _3);
    std::cout << "sizeof(fn): " << sizeof(fn) << std::endl;

    Helloworld h;
    auto       class_fn = boost::bind(&Helloworld::helloworld, &h, _1, _2, _3, _4, _5, 16);
    std::cout << "sizeof(class_fn): " << sizeof(class_fn) << std::endl;

    cout << "Hello World!" << endl;
    if (argc >= 2 && boost::iequals(argv[1], "server")) {
        start_server();
    } else if (argc >= 5 && boost::iequals(argv[1], "client")) {
        start_client(
            argv[2], ::strtoull(argv[3], &argv[3] + strlen(argv[3]) - 1, 10),
            ::strtoull(argv[4], &argv[4] + strlen(argv[4]) - 1, 10));
    } else {
        std::cout << argv[0] << " server/client url count client_count" << std::endl;
    }
    return 0;
}
