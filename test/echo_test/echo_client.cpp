#include <thread>
#include "../../src/client.hpp"

void client_test1() {
    try {
        yarpc::ClientOptions op;
        op.uptime = 5;
        yarpc::Client c(op);
        c.start();

        c.post_message("127.0.0.1", 5678, std::string("hello 1111"));
        c.post_message("127.0.0.1", 5678, std::string("hello 2222"));
        c.post_message("127.0.0.1", 5678, std::string("hello 3333"));
        c.post_message("127.0.0.1", 5678, std::string("hello 4444"));

        c.wait();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

int main() {
    client_test1();
    return 0;
}
