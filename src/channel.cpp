#include "channel.hpp"

namespace yarpc {

using std::string;

void Channel::CallMethod(const google::protobuf::MethodDescriptor *method, google::protobuf::RpcController *controller, 
    const Message *request,Message *response, Closure *done) {
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
