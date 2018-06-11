#include <iostream>
#include "server.hpp"

namespace yarpc {

namespace server_internal {

session::session(boost::asio::io_service& ios, size_t chunk_size) :
    io_service_(ios),
    socket_(ios),
    chunk_size_(chunk_size),
    is_connected_(false),
    error_(false),
    want_close_(false) {
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

        std::shared_ptr<message_op> old_msg_op = msg_writing_q_.front();

        msg_writing_q_.pop_front();
        if (old_msg_op->done) {
            old_msg_op->done();
        }

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

boost::asio::ip::tcp::socket& session::socket() {
    return socket_;
}

void session::start() {
    boost::asio::ip::tcp::no_delay no_delay(true);
    socket_.set_option(no_delay);
    is_connected_ = true;
    read();
}

void session::stop() {
    io_service_.post([this]() {
        want_close_ = true;
        socket_.close();
        is_connected_ = false;
    });
}

void session::read() {
    boost::asio::streambuf::mutable_buffers_type mutable_buff = read_buff_.prepare(chunk_size_);
    socket_.async_read_some(
        mutable_buff,     
        [this, self = shared_from_this()](const boost::system::error_code& err, size_t bytes_transferred) {
            if (!err && bytes_transferred > 0) {
                read_buff_.commit(bytes_transferred);

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

} // namespace

Server::Server(const ServerOptions options) :
    service_poll_sz_(options.service_poll_sz),
    thread_group_sz_(options.thread_group_sz),
    uptime_(options.uptime),
    chunk_size_(options.chunk_size),
    service_pool_(options.service_poll_sz),
    acceptor_(service_pool_.get_io_service()),
    stop_timer_(new boost::asio::steady_timer(service_pool_.get_io_service(0))), 
    want_close_(false),
    protocol_(options.protocol) {

    }

void Server::Start(char const* host, char const* port) {
    auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), atoi(port));
    Start(endpoint);
}

void Server::Start(boost::asio::ip::tcp::endpoint ep) {
    if (uptime_ != -1) {
        stop_timer_->expires_from_now(std::chrono::seconds(uptime_));
        stop_timer_->async_wait([this](const boost::system::error_code& err) {
            Stop();
        });
    }

    acceptor_.open(ep.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(ep);
    acceptor_.listen();
    std::cout << "listen" << std::endl;

    accept();
}

void Server::Stop() {
    for (auto& session : sessions_) {
        session->stop();
    }

    acceptor_.close();
    service_pool_.stop();

    want_close_ = true;
}

void Server::Wait() {
    service_pool_.run();
}

int Server::RegisterService(std::shared_ptr<google::protobuf::Service> service) {
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

std::shared_ptr<google::protobuf::Service> Server::find_service_by_fulllname(std::string full_name) {
    auto it = service_map_.find(full_name);
    return it != service_map_.end() ? it->second : NULL;
}

void Server::send_rpc_reply(gController* controller, const gMessage* response) {
    Controller* cntl = static_cast<Controller*>(controller);
    std::unique_ptr<Controller> recycle_cntl(cntl);

    std::unique_ptr<const gMessage> recycle_res(response);

    std::weak_ptr<server_internal::session> session = recycle_cntl->get_reply_session();
    std::shared_ptr<server_internal::session> session_ptr = session.lock();
    if (session_ptr) {
        string buf_stream;
	    protocol_.serialize_and_packed_response(&buf_stream, cntl, recycle_res.get());

        std::shared_ptr<message_op> msg_op(
            new message_op{
                std::move(buf_stream),
                NULL,
                NULL
            }
        );
        session_ptr->post_msg_queue(msg_op);
    }
}

// private:
void Server::accept() {
    std::shared_ptr<server_internal::session> new_session(
        new server_internal::session(
            service_pool_.get_io_service(), 
            chunk_size_
        )
    );
    std::weak_ptr<server_internal::session> session_ptr(new_session);
    new_session->bind_on_message(
        std::bind(&Server::on_message, this, this, session_ptr, std::placeholders::_1)
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
                std::cout << "accept failed: " << err.message() << "\n";
            }
        }
    });
}

void Server::on_message(Server* serv, 
    std::weak_ptr<server_internal::session> session, 
    boost::asio::streambuf& read_buff) {
        
    // multi protocols
    protocol_.process_and_unpacked_request(serv, session, read_buff);
}

} // namespace 
