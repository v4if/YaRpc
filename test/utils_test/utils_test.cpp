#include <boost/asio.hpp>
#include <iostream>
#include <sstream>

void ep_test() {
    char const* host = "127.0.0.1";
    int port = 80;
    auto endpoint = boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address::from_string(host), port);

    std::cout  << endpoint.address().to_string() << ":" << endpoint.port() << std::endl;
    std::cout << (endpoint.address().to_string() == std::string(host)) << std::endl;
}

void shared_ptr_test() {
    std::shared_ptr<int> sp;
    if (!sp) {
        std::cout << "nullptr" << std::endl;
    }
    // sp.reset(new int(1));
    std::shared_ptr<int> sp1(new int(1));
    sp = sp1;
    std::cout << sp1.use_count() << std::endl;
    std::cout << sp.use_count() << std::endl;

    std::weak_ptr<int> wp(sp);
    auto func = [&sp]() mutable{
        // auto sp_lambda = wp.lock();
        // std::cout << sp_lambda.use_count() << std::endl;
        sp.reset();
        std::cout << sp.use_count() << std::endl;
    };
    func();
    std::cout << sp1.use_count() << std::endl;
    std::cout << sp.use_count() << std::endl;
}

void stdbind_test() {
    class T {
    public: 
        void test(T* t, int data) {
            t->add(data);
            std::cout << data << std::endl;
        }

        void add(int& data) {
            data += 1;
        }
    };

    T t;
    std::function<void(int)> func = std::bind(&T::test, &t, &t, std::placeholders::_1);
    func(1);
}

void string_test() {
    string str;

    size_t size = 778;
    str.append(reinterpret_cast<char*>(&size), sizeof(size));
    cout << size << " hex : " << hex << size << endl;
    cout << str.size() << endl;
    cout << str << endl;

    size_t de_size;
    std::copy(str.begin(), str.begin() + sizeof(size), reinterpret_cast<char*>(&de_size));

    cout << dec << de_size << " hex : " << hex << de_size << endl;
}

int main() {
    stdbind_test();
}
