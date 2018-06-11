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

void Channel::CallMethod(const gMethodDescriptor *method, 
	gController *controller, 
	const gMessage *request, 
	gMessage *response, 
	gClosure *done) {

	Controller* cntl = static_cast<Controller*>(controller);

	string buf_stream;
	uint64 sequence_id = client_->protocol().serialize_and_packed_request(&buf_stream, method, cntl, request);

	// std::cout << "service full name: " << method->service()->full_name() << std::endl;
	// std::cout << "service name: " << method->service()->name() << std::endl;
	// std::cout << "method name: " << method->name() << std::endl;
	// std::cout << "typename: " << request->GetTypeName() << std::endl;

	std::shared_ptr<message_op> msg_op(
		new message_op{
			std::move(buf_stream), 
			response, 
			[done]() {
				// async rpc remote call 
				if (done) {
					done->Run();
				}
			}
		}
	);
	cntl->push_pending_call_map(sequence_id, msg_op);
	client_->post_message(host_, port_, msg_op, cntl);

	// block rpc remote call
	if (!done) {
		msg_op->wait();
	}
}

} // namespace
