#include <iostream>
#include <thread>

#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>

#include "protocol/Protocol.h"
#include "transport/TcpTransport.h"
#include "factory/ClientFactory.h"
#include "BaseAcceptor.h"

using namespace std;

void start_client(int32_t port, int64_t count, int64_t client_count)
{
    boost::asio::io_context ioc;

    std::string host = "10.11.1.147";

    auto client_factory = boost::make_shared<ClientFactory>(ioc);

    // clients
    std::vector<boost::shared_ptr<CBaseProtocol>> clients;
    for (int var = 0; var < client_count; ++ var)
    {
        auto client_instance = client_factory->connect_tcp<Protocol>(host.c_str(), port, 10, 1024);
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
                      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%llu";

    char buffer[2048] = {};

    std::thread thr_ioc([&ioc]() {ioc.run();});
    std::thread thr_writer([&]() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
        for (uint64_t var = 0; true; )
        {
            for (const auto& p: clients)
            {
                if (TcpTransport::EN_OK != p->transport_status())
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
    boost::asio::io_context ioc;

    short port = 8000;
    boost::asio::ip::address address = boost::asio::ip::make_address("0.0.0.0");
    auto client_factory = boost::make_shared<ClientFactory>(ioc);

    // service
    CAcceptor accept(ioc, client_factory);
    accept.listen(boost::asio::ip::tcp::endpoint(address, port));

    ioc.run();
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
        start_client(::atoi(argv[2]), ::atoi(argv[3]), ::atoi(argv[4]));
    }
    else
    {
        std::cout <<  argv[0] << " server/client port count client_count" << std::endl;
    }
    return 0;
}
