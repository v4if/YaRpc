#include "client.hpp"
#include "internal_rpc_protocol.hpp"

namespace yarpc{

namespace client_internal {

stats::stats(int uptime) : uptime_(uptime)
{
}

void stats::add(bool error, std::size_t count_written, std::size_t count_read,
    std::size_t bytes_written, std::size_t bytes_read)
{
    total_error_count_ += error? 1: 0;
    total_count_written_ += count_written;
    total_count_read_ += count_read;
    total_bytes_written_ += bytes_written;
    total_bytes_read_ += bytes_read;
}

void stats::print()
{
    std::cout << total_error_count_ << " total count error\n";
    std::cout << total_count_written_ << " total count written\n";
    std::cout << total_count_read_ << " total count read\n";
    std::cout << total_bytes_written_ << " total bytes written\n";
    std::cout << total_bytes_read_ << " total bytes read\n";
    std::cout << static_cast<double>(total_bytes_read_) /
        (uptime_ * 1024 * 1024) << " MiB/s read throughput\n";
    std::cout << static_cast<double>(total_bytes_written_) /
        (uptime_ * 1024 * 1024) << " MiB/s write throughput\n";

    std::cout << static_cast<double>(total_count_read_) /
        (uptime_) << " counts/s read throughput\n";
    std::cout << static_cast<double>(total_count_written_) /
        (uptime_) << " counts/s written throughput\n";
}

// class yarpc::client_::session
session::session(boost::asio::io_service& ios, boost::asio::ip::tcp::endpoint endpoint, std::size_t chunk_size) :
    io_service_(ios),
    socket_(ios),
    endpoint_(endpoint),
    stop_timer_(new boost::asio::steady_timer(ios)),
    chunk_size_(chunk_size){

}

session::~session() {

}

void session::bind_on_message(std::function<void(boost::asio::streambuf&)> on_message) {
    on_message_ = std::move(on_message);
}

bool session::post_msg_queue(std::shared_ptr<message_op> msg_op) {
    io_service_.post([this, msg_op]() mutable {
        write_one_from_msg_queue(msg_op);
    });
    return true;
}

void session::write_one_from_msg_queue(std::shared_ptr<message_op> msg_op) {
    std::lock_guard<std::mutex> _(mq_writing_mtx_);
    bool in_progress = !msg_writing_q_.empty();
    msg_writing_q_.push_back(std::move(msg_op));
    
    if (is_connected_ && !in_progress) {
        boost::asio::async_write(
            socket_, 
            boost::asio::buffer(msg_writing_q_.front()->message, msg_writing_q_.front()->message.size()),
            [this](const boost::system::error_code& err, std::size_t bytes_transferred) {
                write_until_handler(err, bytes_transferred);
            }
        );
    }
}

void session::write_until_handler(const boost::system::error_code& err, std::size_t bytes_transferred) {
    if (!err) {
        std::lock_guard<std::mutex> _(mq_writing_mtx_);

        bytes_written_ += bytes_transferred;
        ++count_written_;

        std::shared_ptr<message_op> old_msg_op = msg_writing_q_.front();

        msg_writing_q_.pop_front();
        
        if (is_connected_ && !msg_writing_q_.empty()) {
            boost::asio::async_write(
                socket_, 
                boost::asio::buffer(msg_writing_q_.front()->message, msg_writing_q_.front()->message.size()),
                [this](const boost::system::error_code& err, std::size_t bytes_transferred) {
                    write_until_handler(err, bytes_transferred);
                }
            );
        }
    } else {
        if (!want_close_) {
            std::cout << "write failed: " << err.message() << "\n";
            error_ = true;
        }
    }
}

void session::read() {
    boost::asio::streambuf::mutable_buffers_type mutable_buff = read_buff_.prepare(chunk_size_);
    socket_.async_read_some(
        mutable_buff,     
        [this, self = shared_from_this()](const boost::system::error_code& err, size_t bytes_transferred) {
            if (!err && bytes_transferred > 0) {
                read_buff_.commit(bytes_transferred);

                bytes_read_ += bytes_transferred;
                ++count_read_;

                on_message_(read_buff_);
                
                read();
            } else {
                if (!want_close_) { 
                    std::cout << "read failed: " << err.message() << "\n";
                }
            }
        }
    );
}

void session::start(std::shared_ptr<message_op> msg_op) {
    socket_.async_connect(endpoint_, [this, msg_op](const boost::system::error_code& err) mutable {
        if (!err)
        {
            boost::asio::ip::tcp::no_delay no_delay(true);
            socket_.set_option(no_delay);

            is_connected_ = true;
            post_msg_queue(msg_op);

            read();
        }
    });
}

void session::stop() {
    io_service_.post([this]() {
        want_close_ = true;
        socket_.close();
        is_connected_ = false;
    });
}

boost::asio::ip::tcp::endpoint session::remote_point() const {
    return endpoint_;
}

boost::asio::steady_timer& session::get_stop_timer() const {
    return *stop_timer_;
}

std::size_t session::bytes_written() const {
    return bytes_written_;
}

std::size_t session::bytes_read() const {
    return bytes_read_;
}

std::size_t session::count_written() const {
    return count_written_;
}

std::size_t session::count_read() const {
    return count_read_;
}

bool session::error() const {
    return error_;
}

} // namespace

Client::Client(const ClientOptions options) : 
    thread_count_(options.callback_thread_no),
    alive_time_(options.keep_alive_time),
    timeout_(options.time_out),
    uptime_(options.uptime),
    chunk_size_(options.chunk_size),
    io_service_(new boost::asio::io_service),
    io_work_(new boost::asio::io_service::work(*io_service_)),
    resolver_(new boost::asio::ip::tcp::resolver(*io_service_)),
    stats_(options.uptime == -1 ? 1 : options.uptime),
    stop_timer_(new boost::asio::steady_timer(*io_service_)),
    is_running_(false),
    protocol_(options.protocol) {}

Client::~Client() {
    for (auto& session : sessions_) {
        stats_.add(
            session->error(),
            session->count_written(), session->count_read(),
            session->bytes_written(), session->bytes_read());
    }

    stats_.print();
}

void Client::Start() {
    if (uptime_ != -1) {
        stop_timer_->expires_from_now(std::chrono::seconds(uptime_));
        stop_timer_->async_wait([this](const boost::system::error_code& error) {
            Stop();
        });
    }

    threads_.resize(thread_count_);
    for (auto i = 0; i < thread_count_; ++i) {
        threads_[i].reset(new std::thread([this]() {
            try {
                io_service_->run();
            } catch (std::exception& e) {
                std::cerr << "Exception: " << e.what() << "\n";
            }
        }));
    }
    is_running_ = true;
}

void Client::Stop() {
    for (auto& session : sessions_) {
        session->stop();
    }

    io_work_.reset();
    is_running_ = false;
}

bool Client::post_message(char const* host, 
    int port, 
    std::shared_ptr<message_op> msg_op,
    google::protobuf::RpcController* controller) {

    if (!is_running_) return false;

    std::shared_ptr<client_internal::session> ss;
    if (!sessions_.empty()) {
        for (auto& session : sessions_) {
            auto remote = session->remote_point();
            if (remote.address().to_string() == std::string(host) && remote.port() == port) {
                ss = session;
            }
        }
    } 

    if (ss) {
        ss->post_msg_queue(msg_op);
        return true;
    } else {
        boost::asio::ip::tcp::resolver::iterator iter =
            resolver_->resolve(boost::asio::ip::tcp::resolver::query(host, std::to_string(port)));
        boost::asio::ip::tcp::endpoint endpoint = *iter;

        std::shared_ptr<client_internal::session> new_session(
            new client_internal::session(*io_service_, endpoint, chunk_size_)
        );
        new_session->bind_on_message(
            std::bind(&Client::on_message, this, controller, std::placeholders::_1)
        );
        sessions_.emplace_back(new_session);

        if (alive_time_ != -1) {
            boost::asio::steady_timer& stop_timer = new_session->get_stop_timer();
            stop_timer.expires_from_now(std::chrono::seconds(alive_time_));
            stop_timer.async_wait([this, new_session](const boost::system::error_code& error) mutable {
                new_session->stop();
                auto it = sessions_.begin();
                for (;it != sessions_.end();it++) {
                    auto remote = (*it)->remote_point();
                    if (remote.address().to_string() == new_session->remote_point().address().to_string() 
                        && remote.port() == new_session->remote_point().port()) {
                        (*it).reset();
                        sessions_.erase(it);
                        break;
                    }
                }
            });
        }
        new_session->start(msg_op);
        return true;
    }

    return false;
}

void Client::Wait() {
    for (auto& thread : threads_) {
        thread->join();
    }
}

void Client::on_message(google::protobuf::RpcController* controller, boost::asio::streambuf& read_buff) {

    // multi protocols
    protocol_.process_and_unpacked_response(read_buff, controller);

}

protocol_template Client::protocol() {
    return protocol_;
}

bool Client::IsRun() {
    return is_running_;
}

} // namespace 
