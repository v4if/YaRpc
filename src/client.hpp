#ifndef YARPC_CLIENT_HPP_
#define YARPC_CLIENT_HPP_

#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include <mutex>
#include <assert.h>
#include <vector>
#include <memory>
#include <thread>
#include <deque>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>  
#include "io_service_pool.hpp"
#include "message_op.hpp"

namespace yarpc{

namespace client_internal {

class stats
{
public:
    stats(int uptime);

    void add(bool error, std::size_t count_written, std::size_t count_read,
        std::size_t bytes_written, std::size_t bytes_read);

    void print();

private:
    std::size_t total_error_count_ = 0;
    std::size_t total_bytes_written_ = 0;
    std::size_t total_bytes_read_ = 0;
    std::size_t total_count_written_ = 0;
    std::size_t total_count_read_ = 0;
    int uptime_;
};

class session : noncopyable
{
public:
    session(boost::asio::io_service& ios, boost::asio::ip::tcp::endpoint endpoint);

    ~session();

    bool post_msg_queue(std::shared_ptr<message_op> msg_op);      // write 对端消息队列

    void write();

    void read();

    void start(std::shared_ptr<message_op> msg_op);

    void stop();

    boost::asio::ip::tcp::endpoint remote_point() const;

    boost::asio::steady_timer& get_stop_timer() const;

    std::size_t bytes_written() const;

    std::size_t bytes_read() const;

    std::size_t count_written() const;

    std::size_t count_read() const;

    bool error() const;
private:
    void write_one_from_msg_queue(std::shared_ptr<message_op> msg_op);
    void write_until_handler(const boost::system::error_code& err, std::size_t bytes_transferred);

    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::endpoint endpoint_;
    std::unique_ptr<boost::asio::steady_timer> stop_timer_;

    std::mutex mq_mtx;
    std::deque<std::shared_ptr<message_op>> msg_writing_q_;

    std::size_t const block_size_;
    char* const buffer_;
    std::size_t bytes_written_ = 0;
    std::size_t bytes_read_ = 0;
    std::size_t count_written_ = 0;
    std::size_t count_read_ = 0;
    bool is_connected_ = false;
    bool error_ = false;
    bool want_close_ = false;
};

} // namespace

struct ClientOptions {
    int callback_thread_no;     // 回调线程数
    int keep_alive_time;        // session保活时间
    int time_out;               // request超时时间
    int uptime;                 // Client运行时间

    ClientOptions() :
        callback_thread_no(4),
        keep_alive_time(-1),
        time_out(-1),
        uptime(-1)
        {}
};

class Client : noncopyable
{
public:
    Client(const ClientOptions options = ClientOptions());

    ~Client();

    void start();

    void Stop();

    bool post_message(char const* host, int port, std::shared_ptr<message_op> msg_op);

    void wait();

private:
    int const thread_count_;    // 回调线程数
    int const alive_time_;      // session 保活时间
    int const timeout_;         // request 超时时间
    int const uptime_;          // Client 运行时间
    
    std::unique_ptr<boost::asio::io_service> io_service_;
    std::unique_ptr<boost::asio::io_service::work> io_work_;
    std::unique_ptr<boost::asio::ip::tcp::resolver> resolver_;
    std::vector<std::unique_ptr<std::thread>> threads_;

    std::vector<std::shared_ptr<client_internal::session>> sessions_;
    client_internal::stats stats_;

    std::unique_ptr<boost::asio::steady_timer> stop_timer_;

    bool is_running_;
};

} // namespace 

#endif
