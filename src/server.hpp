#ifndef YARPC_SERVER_HPP_
#define YARPC_SERVER_HPP_

#include <thread>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio.hpp>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <map>
#include "noncopyable.hpp"
#include "io_service_pool.hpp"
#include "internal_rpc_protocol.hpp"

namespace yarpc{

namespace server_internal {

// shared_from_this会从weak_ptr安全的生成一个自身的shared_ptr, new创建session, 然后用shared_ptr包裹
class session : public std::enable_shared_from_this<session>, noncopyable
{
public:
    session(boost::asio::io_service& ios, size_t chunk_size_, std::function<void(boost::asio::streambuf&)> on_message);

    ~session();

    boost::asio::ip::tcp::socket& socket();

    void start();

    void stop();

    void write();

    void read();

private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
    std::size_t const chunk_size_;   // 每次read最大bytes

    std::function<void(boost::asio::streambuf&)> on_message_;
    
    // buffers as a pointer and size in bytes
    /**
    typedef std::pair<void*, std::size_t> mutable_buffer;
    typedef std::pair<const void*, std::size_t> const_buffer;
    */
    boost::asio::streambuf read_buff_;  // 读缓冲

    bool is_connected_;
    bool want_close_;
};

} // namespace

struct ServerOptions {
    int service_poll_sz;        // io_service : per cpu
    int thread_group_sz;        // 处理post queue的线程数
    int uptime;                 // 运行时间
    std::size_t chunk_size;             // session 每次read的最大bytes

    ServerOptions() :
        service_poll_sz(std::thread::hardware_concurrency()),
        thread_group_sz(1),
        uptime(-1),
        chunk_size(1024)
        {}
};

class server : noncopyable
{
public:
    server(const ServerOptions options = ServerOptions());

    void start(char const* host, char const* port); 

    void start(boost::asio::ip::tcp::endpoint ep);

    void stop();

    void wait();

    int RegisterService(std::shared_ptr<google::protobuf::Service> service);

private:
    void accept();
    void on_message(server* serv, boost::asio::streambuf& read_buff);

private:
    int const service_poll_sz_;     // io_service 数
    int const thread_group_sz_;     // 处理post queue线程数
    int const uptime_;              // 运行时间
    std::size_t const chunk_size_;       // session 每次read的最大bytes

    io_service_pool service_pool_;
    boost::asio::ip::tcp::acceptor acceptor_;    

    std::vector<std::shared_ptr<server_internal::session>> sessions_;

    std::unique_ptr<boost::asio::steady_timer> stop_timer_;
    bool want_close_;

    std::map<std::string, std::shared_ptr<google::protobuf::Service>> service_map_;
};

} // namespace 

#endif
