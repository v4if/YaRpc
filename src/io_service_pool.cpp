#include <algorithm>
#include <thread>
#include "io_service_pool.hpp"

namespace yarpc {

io_service_pool::io_service_pool(std::size_t pool_size = std::thread::hardware_concurrency()) :
    next_io_service_(0)
{
    pool_size = std::max(pool_size, std::size_t(1));

    // outstanding_work_ + 1, work在, io_service不会退出
    for (std::size_t i = 0; i < pool_size; ++i)
    {
        io_service_ptr io_service(new boost::asio::io_service);
        work_ptr work(new boost::asio::io_service::work(*io_service));
        io_services_.push_back(io_service);
        work_.push_back(work);
    }
}

void io_service_pool::run()
{
    // io_service per cpu 
    std::vector<std::shared_ptr<std::thread> > threads;
    for (std::size_t i = 0; i < io_services_.size(); ++i)
    {
        std::shared_ptr<std::thread> thread(new std::thread(
            [i, this]() {
            io_services_[i]->run();
        }));
        threads.push_back(thread);
    }

    for (std::size_t i = 0; i < threads.size(); ++i)
        threads[i]->join();
}

void io_service_pool::stop()
{
    // reset work
    for (std::size_t i = 0; i < work_.size(); ++i)
        work_[i].reset();
}

std::size_t io_service_pool::get_poll_size() {
    return io_services_.size();
}

boost::asio::io_service& io_service_pool::get_io_service()
{
    // round-robin
    boost::asio::io_service& io_service = *io_services_[next_io_service_];
    ++next_io_service_;
    if (next_io_service_ == io_services_.size())
        next_io_service_ = 0;
    return io_service;
}

boost::asio::io_service& io_service_pool::get_io_service(size_t index) {
    index = index % io_services_.size();
    return *io_services_[index];
}

} // namespace
