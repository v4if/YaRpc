#ifndef YARPC_CONTROLLER_HPP_
#define YARPC_CONTROLLER_HPP_

#include <google/protobuf/service.h>
#include <string>
#include <deque>
#include "port.hpp"
#include "message_op.hpp"

namespace yarpc {

using std::string;
class Controller : public google::protobuf::RpcController {
public:
    Controller() : sequence_id_(0) { Reset(); }

    virtual ~Controller() {}

    // Resets the RpcController to its initial state so that it may be reused in a new call
    virtual void Reset() {
        error_text_ = "";
        is_failed_ = false;
    }

    // After a call has finished, returns true if the call failed
    virtual bool Failed() const {
        return is_failed_;
    }

    // If Failed() is true, returns a human-readable description of the error
    virtual string ErrorText() const {
        return error_text_;
    }

    // Advises the RPC system that the caller desires that the RPC call be canceled
    virtual void StartCancel() { // NOT IMPL
        return ;
    }

    // Causes Failed() to return true on the client side
    void SetFailed(const string &reason) {
        is_failed_ = true;
        error_text_ = reason;
    }

    // If true, indicates that the client canceled the RPC, so the server may as well give up on replying to it
    bool IsCanceled() const { // NOT IMPL
        return false;
    }
    
    // Asks that the given callback be called when the RPC is canceled
    void NotifyOnCancel(google::protobuf::Closure* callback) { // NOT IMPL
        return;
    }

    // ==================================================
    void push_pending_q(std::shared_ptr<message_op> msg) {
        msg_pending_q_.push_back(msg);
    }

    uint64 next_sequence_id() {
        if (sequence_id_ == std::numeric_limits<uint64_t>::max()) {
            sequence_id_ = 0;
        }
        return ++sequence_id_;
    }

private:
    string error_text_;
    bool is_failed_;

    std::deque<std::shared_ptr<message_op>> msg_pending_q_;
    uint64 sequence_id_;
};

} // namespace yarpc

#endif
