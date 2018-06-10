#ifndef YARPC_MESSAGE_OP_HPP_
#define YARPC_MESSAGE_OP_HPP_

#include <string>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

namespace yarpc {

enum future_status {
    FUTURE_PENDING,     // 未处理
    FUTURE_READY,       // 处理完成
    FUTURE_TIME_OUT     // 超时
};

class message_op {
public:
    std::string message;
    // has not ownship
    google::protobuf::Message* response;
    std::function<void(void)> done;

    message_op(std::string msg, google::protobuf::Message* res, std::function<void(void)> d) :
        message(msg),
        response(res),
        done(d),
        status(FUTURE_PENDING) {

    }
    message_op(const message_op& other) :
        message(other.message),
        response(other.response),
        done(other.done),
        status(other.status) {

    }

    void wait() {
        std::unique_lock<std::mutex> _(mtx_);
        // ref
        cond_.wait(_, [this]{return status != FUTURE_PENDING;});
    }

    void notify() {
        if (status == FUTURE_PENDING) {
            std::unique_lock<std::mutex> _(mtx_);
            if (status == FUTURE_PENDING) {
                status = FUTURE_READY;
                cond_.notify_all();
            }
        }
    }

private:
    std::mutex mtx_;
    std::condition_variable cond_;
    future_status status;
};

};

#endif
