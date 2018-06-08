#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <vector>
#include <unistd.h>

namespace test1 {
// 线程不安全自增
struct Test {
    static long long data;
    void thread_add()
    {
        for (int i = 0;i < 100000;i++) {
            data++;
        }
    } 
};
long long Test::data = 0;
void test1() {
    Test test;
    int thread_no = 10;
    std::vector<std::shared_ptr<boost::thread>> v;
    for (int i = 0;i < thread_no;i++) {
        std::shared_ptr<boost::thread> temp(new boost::thread(boost::bind(&Test::thread_add, &test)));
        v.push_back(temp);
    }

    for (int i = 0;i < thread_no;i++) {
        v[i]->join();
    }

    std::cout << Test::data << std::endl;
}
} // namespace test1

namespace test2 {
struct Test {
    static long long data;
    boost::mutex mt;
    void thread_add()
    {
        for (int i = 0;i < 100000;i++) {
            data++;
        }
    } 
    void thread_safe_add() {
        for (int i = 0;i < 100000;i++) {
            boost::lock_guard<boost::mutex> _(mt);
            data++;
        }
    }
};
long long Test::data = 0;

void test2() {
    boost::asio::io_service io_service;

    Test test;
    int post_no = 2;
    for (int i = 0;i < post_no;i++) {
        io_service.post(boost::bind(&Test::thread_add, &test));
    }

    int thread_no = 10;
    std::vector<std::shared_ptr<boost::thread>> v;
    for (int i = 0;i < thread_no;i++) {
        std::shared_ptr<boost::thread> temp(
            new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service))
        );
        v.push_back(temp);
    }
    
    for (int i = 0;i < thread_no;i++) {
        v[i]->join();
    }

    std::cout << Test::data << std::endl;
}
} // namespace test2

namespace test3 {
// 成员函数bind test
void test3() {
    test2::Test test;
    auto func = std::bind(&test2::Test::thread_add, &test);
    func();
    std::cout << test2::Test::data << std::endl;
}
} // namespace test3

namespace test4 {
// thread safe test
void test() {
    boost::asio::io_service io_service;

    test2::Test test;
    int post_no = 5;
    for (int i = 0;i < post_no;i++) {
        io_service.post(boost::bind(&test2::Test::thread_safe_add, &test));
    }

    int thread_no = 10;
    std::vector<std::shared_ptr<boost::thread>> v;
    for (int i = 0;i < thread_no;i++) {
        std::shared_ptr<boost::thread> temp(
            new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service))
        );
        v.push_back(temp);
    }
    
    for (int i = 0;i < thread_no;i++) {
        v[i]->join();
    }

    std::cout << test2::Test::data << std::endl;
}
} // namespace test4

int main(int argc, char* argv[])
{
    test4::test();
}
