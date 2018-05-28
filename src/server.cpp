#include <iostream>
#include "server.hpp"

namespace yarpc {

namespace server_internal {

session::session(boost::asio::io_service& ios, size_t block_size) :
    io_service_(ios),
    socket_(ios),
    block_size_(block_size),
    buffer_(new char[block_size]),
    is_connected_(false),
    want_close_(false) {
}

session::~session() {
    delete[] buffer_;
}

boost::asio::ip::tcp::socket& session::socket() {
    return socket_;
}

void session::start() {
    boost::asio::ip::tcp::no_delay no_delay(true);
    socket_.set_option(no_delay);
    read();
}

void session::stop() {
    io_service_.post([this]() {
        want_close_ = true;
        socket_.close();
        is_connected_ = false;
    });
}

void session::write() {
    boost::asio::async_write(socket_, boost::asio::buffer(buffer_, block_size_),
        [this, self = shared_from_this()](
            const boost::system::error_code& err, size_t bytes_transferred) {
        if (!err) {
            assert(bytes_transferred == block_size_);
            read();
        }
    });
}

void session::read() {
    boost::asio::async_read(socket_, boost::asio::buffer(buffer_, 10),
        [this, self = shared_from_this()](
            const boost::system::error_code& err, size_t bytes_transferred) {
        if (!err) {
            // assert(bytes_transferred == block_size_);
            std::cout << std::string(buffer_, bytes_transferred) << std::endl;
            // write();
            read();
        } else {
            if (!want_close_) {
                std::cout << "read failed: " << err.message() << "\n";
            }
        }
    });
}

} // namespace

server::server(const ServerOptions options) :
    service_poll_sz_(options.service_poll_sz),
    thread_group_sz_(options.thread_group_sz),
    uptime_(options.uptime),
    block_size_(1024),
    service_pool_(options.service_poll_sz),
    acceptor_(service_pool_.get_io_service()),
    stop_timer_(new boost::asio::steady_timer(service_pool_.get_io_service(0))), 
    want_close_(false) {

    }

void server::start(boost::asio::ip::tcp::endpoint ep) {
    if (uptime_ != -1) {
        stop_timer_->expires_from_now(std::chrono::seconds(uptime_));
        stop_timer_->async_wait([this](const boost::system::error_code& err) {
            stop();
        });
    }

    acceptor_.open(ep.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(ep);
    acceptor_.listen();
    std::cout << "listen" << std::endl;

    accept();
}

void server::stop() {
    for (auto& session : sessions_) {
        session->stop();
    }

    acceptor_.close();
    service_pool_.stop();

    want_close_ = true;
}

void server::wait() {
    service_pool_.run();
}

// private:
void server::accept() {
    std::shared_ptr<server_internal::session> new_session(new server_internal::session(service_pool_.get_io_service(), block_size_));
    sessions_.emplace_back(new_session);

    std::weak_ptr<server_internal::session> wp(new_session);

    auto& socket = new_session->socket();
    acceptor_.async_accept(socket,
        [this, wp](const boost::system::error_code& err) {
        if (!err) {
            std::shared_ptr<server_internal::session> sp = wp.lock();
            if (sp) {
                sp->start();
            }
            
            accept();
        } else {
            if (!want_close_) {
                std::cout << "accept failed: " << err.message() << "\n";
            }
        }
    });
}

} // namespace 