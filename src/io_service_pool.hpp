#ifndef YARPC_IO_SERVICE_POOL_HPP_
#define YARPC_IO_SERVICE_POOL_HPP_

#include <boost/asio.hpp>
#include "noncopyable.hpp"

namespace yarpc {

class io_service_pool : noncopyable {
public:
    explicit io_service_pool(std::size_t pool_size);

    void run();

    void stop();

    std::size_t get_poll_size();

    boost::asio::io_service& get_io_service();

    boost::asio::io_service& get_io_service(size_t index);
private:
    typedef std::shared_ptr<boost::asio::io_service> io_service_ptr;
    typedef std::shared_ptr<boost::asio::io_service::work> work_ptr;

    std::vector<io_service_ptr> io_services_;

    std::vector<work_ptr> work_;

// 为了实现round-robin 指向下一个io_service的index
    std::size_t next_io_service_;
};

} // namespace

#endif
