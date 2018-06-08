#include <iostream>
#include "server.hpp"

namespace yarpc {

namespace server_internal {

session::session(boost::asio::io_service& ios, 
    size_t chunk_size, 
    std::function<void(boost::asio::streambuf&)> on_message) :
    io_service_(ios),
    socket_(ios),
    chunk_size_(chunk_size),
    on_message_(on_message),
    is_connected_(false),
    want_close_(false) {
}

session::~session() {

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
    // boost::asio::async_write(socket_, boost::asio::buffer(buff, max_size_),
    //     [this, self = shared_from_this()](
    //         const boost::system::error_code& err, size_t bytes_transferred) {
    //     if (!err) {
    //         assert(bytes_transferred == block_size_);
    //         read();
    //     }
    // });
}

void session::read() {
    boost::asio::streambuf::mutable_buffers_type mutable_buff = read_buff_.prepare(chunk_size_);
    socket_.async_read_some(mutable_buff,     
        [this, self = shared_from_this()](const boost::system::error_code& err, 
            size_t bytes_transferred) {
        if (!err && bytes_transferred > 0) {
            read_buff_.commit(bytes_transferred);

            on_message_(read_buff_);
            
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
    chunk_size_(options.chunk_size),
    service_pool_(options.service_poll_sz),
    acceptor_(service_pool_.get_io_service()),
    stop_timer_(new boost::asio::steady_timer(service_pool_.get_io_service(0))), 
    want_close_(false) {

    }

void server::start(char const* host, char const* port) {
    auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), atoi(port));
    start(endpoint);
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

int server::RegisterService(std::shared_ptr<google::protobuf::Service> service) {
    const google::protobuf::ServiceDescriptor* sd = service->GetDescriptor();
    if (sd->method_count() == 0) {
        // service does not have any method
        return -1;
    }

    if (service_map_.find(sd->full_name()) != service_map_.end()) {
        // service already exists
        return -1;
    }

    service_map_[sd->full_name()] = service;
    return 0;
}

std::shared_ptr<google::protobuf::Service> server::find_service_by_fulllname(std::string full_name) {
    auto it = service_map_.find(full_name);
    return it != service_map_.end() ? it->second : NULL;
}

void server::send_rpc_reply() {
    std::cout << "reply" << std::endl;
}

// private:
void server::accept() {
    std::shared_ptr<server_internal::session> new_session(
        new server_internal::session(
            service_pool_.get_io_service(), 
            chunk_size_, 
            std::bind(&server::on_message, this, this, std::placeholders::_1)
        )
    );
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
                std::cout << "server.cpp accept failed: " << err.message() << "\n";
            }
        }
    });
}

void server::on_message(server* serv, boost::asio::streambuf& read_buff) {
    // multi protocols
    internal_rpc_protocol::process_and_unpacked_request(serv, read_buff);
}

} // namespace 
