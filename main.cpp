#include <iostream>
#include <thread>
#include <inttypes.h>
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>

#include "protocol/GenericProtocol.h"
#include "transport/TcpTransport.h"
#include "factory/ClientFactory.h"
#include "listener/TcpListener.h"

using namespace std;
#ifndef PRIu64
#  define __PRI_64_LENGTH_MODIFIER__ "ll"
#  define PRIu64        __PRI_64_LENGTH_MODIFIER__ "u"
#endif

void start_client(int32_t port, uint64_t count, int64_t client_count)
{
    auto ioc = boost::make_shared<boost::asio::io_context>();

    std::string host = "10.11.1.147";

    auto client_factory = boost::make_shared<ClientFactory>(ioc);

    // clients
    std::vector<boost::shared_ptr<BaseProtocol>> clients;
    for (int var = 0; var < client_count; ++ var)
    {
        auto client_instance = client_factory->connect_tcp<GenericProtocol>(host.c_str(), port, 10, 1024);
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
                    ::sleep(1);
                    continue;
                }

                std::sprintf(buffer, fmt, var);
                p->write(buffer);

            }
            if ((var % count) == 0)
            {
                ::sleep(1);
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

void start_server()
{
    auto ioc = boost::make_shared<boost::asio::io_context>();

    uint16_t port = 8000;
    boost::asio::ip::address address = boost::asio::ip::make_address("0.0.0.0");
    auto client_factory = boost::make_shared<ClientFactory>(ioc);

    // service
    TcpListener listener;
    listener.listen<GenericProtocol>(
                ioc,
                boost::asio::ip::tcp::endpoint(address, port),
                client_factory
                );

    ioc->run();
}

int main(int argc, char *argv[])
{
    cout << "Hello World!" << endl;
    if (argc >= 2 && 0 == strcasecmp(argv[1], "server"))
    {
        start_server();
    }
    else if (argc >= 5 && 0 == strcasecmp(argv[1], "client"))
    {
        start_client(::atoi(argv[2]), ::strtoull(argv[3], &argv[3]+strlen(argv[3]), 10), ::atoi(argv[4]));
    }
    else
    {
        std::cout <<  argv[0] << " server/client port count client_count" << std::endl;
    }
    return 0;
}
