#include "channel.hpp"
#include "controller.hpp"
#include "message_op.hpp"
#include "internal_rpc_protocol.hpp"

namespace yarpc {

using std::string;

Channel::Channel(Client* clt, char const* host, int const port) :
	client_(clt),
	host_(host),
	port_(port) {

}

Channel::~Channel() {}

void Channel::CallMethod(const gMethodDescriptor *method, gController *controller, 
        const gMessage *request, gMessage *response, gClosure *done) {
	Controller* cntl = static_cast<Controller*>(controller);

	string buf_stream;
	internal_rpc_protocol::serialize_and_packed_request(&buf_stream, method, cntl, request);

	std::cout << "service full name: " << method->service()->full_name() << std::endl;
	std::cout << "service name: " << method->service()->name() << std::endl;
    std::cout << "method name: " << method->name() << std::endl;
	std::cout << "typename: " << request->GetTypeName() << std::endl;

	std::shared_ptr<message_op> msg_op(new message_op{std::move(buf_stream), [msg_op, done]() {
		std::cout << "done" << std::endl;
		msg_op->notify();
		// 异步请求
		if (done) {
			done->Run();
		}
	}});
	cntl->push_pending_q(msg_op);
	client_->post_message(host_, port_, msg_op);

	// block rpc remote call
	if (!done) {
		msg_op->wait();
	}

		// request->SerializeAsString();
	// if (!_session_id) {
	// 	Connect(controller);
	// 	if (controller->Failed()) return;
	// }

	// const string &service_name = method->service()->name();
	
	
	// std::string * content =  new std::string;
	// request->SerializeToString(content);
	// _client->CallMsgEnqueue(_session_id, content, service_id, method->index(),
	// 	controller, response, done, _write_pipe);
	
	// if (!done) {
	// 	char buf;
	// 	read(_read_pipe, &buf, sizeof(buf));
	// }
}

} // namespace
