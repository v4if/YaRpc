#ifndef yarpc_controller_hpp_
#define yarpc_controller_hpp_

#include <google/protobuf/service.h>
#include <string>

namespace yarpc {

using std::string;
class Controller : public google::protobuf::RpcController {
public:
    Controller() { Reset(); }

    virtual ~Controller() {}

    // Resets the RpcController to its initial state so that it may be reused in a new call
    virtual void Reset() {
        _error_text = "";
        _is_failed = false;
    }

    // After a call has finished, returns true if the call failed
    virtual bool Failed() const {
        return _is_failed;
    }

    // If Failed() is true, returns a human-readable description of the error
    virtual string ErrorText() const {
        return _error_text;
    }

    // Advises the RPC system that the caller desires that the RPC call be canceled
    virtual void StartCancel() { // NOT IMPL
        return ;
    }

    // Causes Failed() to return true on the client side
    void SetFailed(const string &reason) {
        _is_failed = true;
        _error_text = reason;
    }

    // If true, indicates that the client canceled the RPC, so the server may as well give up on replying to it
    bool IsCanceled() const { // NOT IMPL
        return false;
    }
    
    // Asks that the given callback be called when the RPC is canceled
    void NotifyOnCancel(google::protobuf::Closure* callback) { // NOT IMPL
        return;
    }

private:
    string _error_text;
    bool _is_failed;
};

} // namespace yarpc

#endif
