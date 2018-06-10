#ifndef YARPC_CONTROLLER_HPP_
#define YARPC_CONTROLLER_HPP_

#include <google/protobuf/service.h>
#include <string>
#include <map>
#include "port.hpp"
#include "message_op.hpp"

namespace yarpc {

class server;
namespace server_internal {
    class session;
}

using std::string;
class Controller : public google::protobuf::RpcController {
public:
    Controller();

    virtual ~Controller();

    // Resets the RpcController to its initial state so that it may be reused in a new call
    virtual void Reset();

    // After a call has finished, returns true if the call failed
    virtual bool Failed() const;

    // If Failed() is true, returns a human-readable description of the error
    virtual string ErrorText() const;

    // Advises the RPC system that the caller desires that the RPC call be canceled
    virtual void StartCancel();

    // Causes Failed() to return true on the client side
    void SetFailed(const string &reason);

    // If true, indicates that the client canceled the RPC, so the server may as well give up on replying to it
    bool IsCanceled() const;
    
    // Asks that the given callback be called when the RPC is canceled
    void NotifyOnCancel(google::protobuf::Closure* callback);

    // ==================================================
    uint64 next_sequence_id();

    void set_reply_sequence_id(uint64 sequence_id);

    uint64 get_reply_sequence_id();

    void clear_reply_sequence_id(uint64 sequence_id);

    void set_reply_session(std::weak_ptr<server_internal::session> session);

    std::weak_ptr<server_internal::session> get_reply_session();

    void clear_reply_session();

    bool push_pending_call_map(uint64 sequence_id, std::shared_ptr<message_op> msg_op);

    std::shared_ptr<message_op> find_pending_call_by_sequenceid(uint64 sequence_id);

    void remove_pending_call(uint64 sequence_id);

private:
    string error_text_;
    bool is_failed_;

    uint64 request_sequence_id_;
    uint64 response_sequence_id_;

    std::weak_ptr<server_internal::session> reply_session_;

    // client pending call 未决状态的rpc call
    // <sequence_id, message_op>
    std::map<uint64, std::shared_ptr<message_op>> pending_call_map_;
};

} // namespace yarpc

#endif
