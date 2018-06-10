#include "controller.hpp"
#include "server.hpp"

namespace yarpc {

Controller::Controller() : request_sequence_id_(0), response_sequence_id_(0) { Reset(); }

Controller::~Controller() {}

// Resets the RpcController to its initial state so that it may be reused in a new call
void Controller::Reset() {
    error_text_ = "";
    is_failed_ = false;
}

// After a call has finished, returns true if the call failed
bool Controller::Failed() const {
    return is_failed_;
}

// If Failed() is true, returns a human-readable description of the error
string Controller::ErrorText() const {
    return error_text_;
}

// Advises the RPC system that the caller desires that the RPC call be canceled
void Controller::StartCancel() { // NOT IMPL
    return ;
}

// Causes Failed() to return true on the client side
void Controller::SetFailed(const string &reason) {
    is_failed_ = true;
    error_text_ = reason;
}

// If true, indicates that the client canceled the RPC, so the server may as well give up on replying to it
bool Controller::IsCanceled() const { // NOT IMPL
    return false;
}

// Asks that the given callback be called when the RPC is canceled
void Controller::NotifyOnCancel(google::protobuf::Closure* callback) { // NOT IMPL
    return;
}

// ==================================================
uint64 Controller::next_sequence_id() {
    if (request_sequence_id_ == std::numeric_limits<uint64_t>::max()) {
        request_sequence_id_ = 0;
    }
    return ++request_sequence_id_;
}

void Controller::set_reply_sequence_id(uint64 sequence_id) {
    response_sequence_id_ = sequence_id;
}

uint64 Controller::get_reply_sequence_id() {
    return response_sequence_id_;
}

void Controller::clear_reply_sequence_id(uint64 sequence_id) {
    response_sequence_id_ = 0;
}

void Controller::set_reply_session(std::weak_ptr<server_internal::session> session) {
    reply_session_ = session;
}

std::weak_ptr<server_internal::session> Controller::get_reply_session() {
    return reply_session_;
}

void Controller::clear_reply_session() {
    reply_session_.reset();
}

bool Controller::push_pending_call_map(uint64 sequence_id, std::shared_ptr<message_op> msg_op) {
    if (pending_call_map_.find(sequence_id) != pending_call_map_.end()) {
        return false;
    }
    pending_call_map_[sequence_id] = msg_op;
    return true;
}

std::shared_ptr<message_op> Controller::find_pending_call_by_sequenceid(uint64 sequence_id) {
    auto it = pending_call_map_.find(sequence_id);
    return it != pending_call_map_.end() ? it->second : NULL;
}

void Controller::remove_pending_call(uint64 sequence_id) {
    auto it = pending_call_map_.find(sequence_id);
    if (it != pending_call_map_.end()) {
        pending_call_map_.erase(it);
    }
}

} // namespace yarpc
