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
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

#include "protocol_template.hpp"
#include "internal_rpc_protocol.hpp"
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

class session : public std::enable_shared_from_this<session>, noncopyable
{
public:
    session(boost::asio::io_service& ios, boost::asio::ip::tcp::endpoint endpoint, std::size_t chunk_size);

    ~session();

    void bind_on_message(std::function<void(boost::asio::streambuf&)> on_message);

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

    std::mutex mq_writing_mtx_;
    std::deque<std::shared_ptr<message_op>> msg_writing_q_;

    std::size_t const chunk_size_;   // 每次read最大bytes
    boost::asio::streambuf read_buff_;  // 读缓冲
    std::function<void(boost::asio::streambuf&)> on_message_;

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
    std::size_t chunk_size;         // session 每次read的最大bytes
    protocol_template protocol;     // 协议

    ClientOptions() :
        callback_thread_no(4),
        keep_alive_time(-1),
        time_out(-1),
        uptime(-1),
        chunk_size(1024),
        protocol{
            internal_rpc_protocol::serialize_and_packed_request,
            internal_rpc_protocol::process_and_unpacked_response,
            internal_rpc_protocol::serialize_and_packed_response,
            internal_rpc_protocol::process_and_unpacked_request,
        }
        {}
};

class Client : noncopyable
{
public:
    Client(const ClientOptions options = ClientOptions());

    ~Client();

    void Start();

    void Stop();

    bool post_message(
        char const* host, 
        int port, 
        std::shared_ptr<message_op> msg_op, 
        google::protobuf::RpcController* controller);

    void Wait();

    protocol_template protocol();

private:
    void on_message(google::protobuf::RpcController* controller, boost::asio::streambuf& read_buff);

    int const thread_count_;        // 回调线程数
    int const alive_time_;          // session 保活时间
    int const timeout_;             // request 超时时间
    int const uptime_;              // Client 运行时间
    std::size_t const chunk_size_;         // session 每次read的最大bytes
    
    std::unique_ptr<boost::asio::io_service> io_service_;
    std::unique_ptr<boost::asio::io_service::work> io_work_;
    std::unique_ptr<boost::asio::ip::tcp::resolver> resolver_;
    std::vector<std::unique_ptr<std::thread>> threads_;

    std::vector<std::shared_ptr<client_internal::session>> sessions_;
    client_internal::stats stats_;

    std::unique_ptr<boost::asio::steady_timer> stop_timer_;

    bool is_running_;

    protocol_template protocol_;
};

} // namespace 

#endif
