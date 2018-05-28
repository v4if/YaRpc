#include <boost/asio.hpp>
#include <thread>
#include "../../src/server.hpp"

void server_test1(int thread_count, char const* host, char const* port,
    std::size_t block_size) {
    auto endpoint = boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address::from_string(host), atoi(port));
    
    yarpc::ServerOptions options;
    options.uptime = 8;
    // 要保证server的生命周期, server里面有回调, server在, io_service_poll就在
    yarpc::server s(options);
    s.start(endpoint);
    s.wait();
}

int main() {
    server_test1(1, "127.0.0.1", "5678", 1024);
    return 0;
}
