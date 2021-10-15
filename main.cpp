#include <iostream>
#include <thread>
#include <inttypes.h>
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>

#include "factory/AutoConnector.h"
#include "listener/Listener.h"
#include "listener/TcpListener.h"
#include "protocol/GenericProtocol.h"
#include "transport/TcpTransport.h"

using namespace std;
#ifndef PRIu64
#  define __PRI_64_LENGTH_MODIFIER__ "ll"
#  define PRIu64        __PRI_64_LENGTH_MODIFIER__ "u"
#endif

namespace fn {
void sleep(long sec)
{
    boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(sec));
}

void msleep(long millisec)
{
    boost::thread::sleep(boost::get_system_time() + boost::posix_time::millisec(millisec));
}
}

boost::system_time future(long sec)
{
    return boost::get_system_time() + boost::posix_time::seconds(sec);
}

void start_client(const std::string &url, uint64_t count, uint64_t client_count)
{
    auto ioc = boost::make_shared<boost::asio::io_context>();

    auto reconnect_factory = boost::make_shared<AutoReconnector>(ioc);

    // clients
    std::vector<boost::shared_ptr<BaseProtocol>> clients;
    for (int var = 0; var < client_count; ++ var)
    {
        auto client_instance = reconnect_factory->connect<GenericProtocol, TcpTransport>(url, 10, 1024);
        if (client_instance)
            clients.push_back(client_instance);
    }

    const char *fmt = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
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

    char buffer[2048] = {};

    std::thread thr_ioc([&ioc]() {ioc->run();});
    std::thread thr_writer([&]() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
        for (uint64_t var = 0; true; )
        {
            for (const auto& p: clients)
            {
                if (transport::EN_OK != p->transport_status())
                {
                    fn::sleep(1);
                    continue;
                }

                std::sprintf(buffer, fmt, var);
                p->write(buffer);

            }
            if ((var % count) == 0)
            {
                fn::sleep(1);
                std::cout << "send count: " << var << " * " << client_count << std::endl;
            }
            ++ var;
        }
#pragma clang diagnostic pop
    });

    // wait the thread stopped
    thr_writer.join();
    thr_ioc.join();

    // close clients
    for (auto p: clients)
    {
        p->close();
    }
}

void start_server() {
#if 1
    auto ioc = boost::make_shared<boost::asio::io_context>();

    uint16_t                 port              = 8000;
    boost::asio::ip::address address           = boost::asio::ip::make_address("0.0.0.0");
    auto                     reconnect_factory = boost::make_shared<AutoReconnector>(ioc);

    // service

#if 0
    TcpListener listener;
    listener.listen<GenericProtocol>(ioc, boost::asio::ip::tcp::endpoint(address, port), reconnect_factory);
#else
    TcpTransport transport(ioc, 100, 1024);
    transport.accept("tcp://127.0.0.1:8888");
#endif
    ioc->run();
#endif
}

class Helloworld
{
public:
    void helloworld(int i1, int i2, int i3, int i4, int i5, int i6) {
        std::cout << i1 + i2 + i3 + i4 << std::endl;
    }
};

int main(int argc, char *argv[])
{
    auto fn = boost::bind(start_client, _1, _2, _3);
    std::cout << "sizeof(fn): " <<  sizeof(fn) << std::endl;

    Helloworld h;
    auto class_fn = boost::bind(&Helloworld::helloworld, &h, _1, _2, _3, _4, _5, 16);
    std::cout << "sizeof(class_fn): " <<  sizeof(class_fn) << std::endl;

    cout << "Hello World!" << endl;
    if (argc >= 2 && boost::iequals(argv[1], "server"))
    {
        start_server();
    }
    else if (argc >= 5 && boost::iequals(argv[1], "client"))
    {
        start_client(
            argv[2],
            ::strtoull(argv[3], &argv[3]+strlen(argv[3]) - 1, 10),
            ::strtoull(argv[4], &argv[4]+strlen(argv[4]) - 1, 10)
            );
    }
    else
    {
        std::cout <<  argv[0] << " server/client port count client_count" << std::endl;
    }
    return 0;
}
