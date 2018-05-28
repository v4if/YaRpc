#ifndef yarpc_server_hpp_
#define yarpc_server_hpp_

#include <thread>
#include <boost/asio/steady_timer.hpp>
#include "noncopyable.hpp"
#include "io_service_pool.hpp"

namespace yarpc{

namespace server_internal {

// shared_from_this会从weak_ptr安全的生成一个自身的shared_ptr, new创建session, 然后用shared_ptr包裹
class session : public std::enable_shared_from_this<session>, noncopyable
{
public:
    session(boost::asio::io_service& ios, size_t block_size);

    ~session();

    boost::asio::ip::tcp::socket& socket();

    void start();

    void stop();

    void write();

    void read();

private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
    size_t const block_size_;
    char* buffer_;

    bool is_connected_;
    bool want_close_;
};

} // namespace

struct ServerOptions {
    int service_poll_sz;        // io_service : per cpu
    int thread_group_sz;        // 处理post queue的线程数
    int uptime;                 // 运行时间

    ServerOptions() :
        service_poll_sz(std::thread::hardware_concurrency()),
        thread_group_sz(1),
        uptime(-1)
        {}
};

class server : noncopyable
{
public:
    server(const ServerOptions options = ServerOptions());

    void start(boost::asio::ip::tcp::endpoint ep);

    void stop();

    void wait();

private:
    void accept();

private:
    int const service_poll_sz_;     // io_service 数
    int const thread_group_sz_;     // 处理post queue线程数
    int const uptime_;              // 运行时间
    size_t const block_size_;
    io_service_pool service_pool_;
    boost::asio::ip::tcp::acceptor acceptor_;    

    std::vector<std::shared_ptr<server_internal::session>> sessions_;

    std::unique_ptr<boost::asio::steady_timer> stop_timer_;
    bool want_close_;
};

} // namespace 

#endif
