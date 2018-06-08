/**
Boost::asio io_service 实现分析 io_service的作用 io_servie 实现了一个任务队列，这里的任务就是void(void)的函数。
Io_servie最常用的两个接口是post和run，post向任务队列中投递任务，run是执行队列中的任务，直到全部执行完毕，并且run可以被N个线程调用。
*/
#include <boost/asio.hpp>   
#include <boost/thread.hpp>   
#include <iostream>   
#include <unistd.h>
#include <ctime>
#include <ratio>
#include <chrono>

void timenow(std::string&& desc) {
    using namespace std::chrono;

    duration<int,std::ratio<60*60*24> > one_day (1);

    system_clock::time_point now = system_clock::now();

    time_t tt;

    tt = system_clock::to_time_t ( now );
    std::cout << desc << " : " << ctime(&tt);
}

void handler1(const boost::system::error_code &ec)   
{   
    timenow(std::string{"handler1"});
    std::cout <<  "handler1 thread id : " << boost::this_thread::get_id() << std::endl;
}   

void handler2(const boost::system::error_code &ec)   
{   
    timenow(std::string{"handler2"});
    // sleep(3);
    std::cout <<  "handler2 thread id : " << boost::this_thread::get_id() << std::endl; 
}   

boost::asio::io_service io_service;   

void run()   
{   
    io_service.run();   
}   

int main()
{   
    boost::asio::deadline_timer timer1(io_service, boost::posix_time::seconds(2));   
    timer1.async_wait(handler1);   
    boost::asio::deadline_timer timer2(io_service, boost::posix_time::seconds(2));   
    timer2.async_wait(handler2);  

    timenow(std::string{"run"});

    // 多个线程对同一个资源对象 std::cout 进行操作，会出现串流，需要加同步
    boost::thread thread1(run);   
    boost::thread thread2(run);   
    thread1.join();   
    thread2.join();   

    timenow(std::string{"done"});
}
