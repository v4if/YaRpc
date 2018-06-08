#include <boost/asio.hpp>
#include <thread>
#include "../../src/server.hpp"
#include "echo.pb.h"

class EchoServiceImpl : public example::EchoService
{
public:
    EchoServiceImpl() {}
    virtual ~EchoServiceImpl() {}

private:
    virtual void Echo(google::protobuf::RpcController* controller,
                      const example::EchoRequest* request,
                      example::EchoResponse* response,
                      google::protobuf::Closure* done)
    {
        // sofa::pbrpc::RpcController* cntl = static_cast<sofa::pbrpc::RpcController*>(controller);
        // SLOG(INFO, "Echo(): request message from %s: %s",
        //         cntl->RemoteAddress().c_str(), request->message().c_str());
        // if (cntl->IsHttp()) {
        //     SLOG(INFO, "HTTP-PATH=\"%s\"", cntl->HttpPath().c_str());
        //     std::map<std::string, std::string>::const_iterator it;
        //     const std::map<std::string, std::string>& query_params = cntl->HttpQueryParams();
        //     for (it = query_params.begin(); it != query_params.end(); ++it) {
        //         SLOG(INFO, "QueryParam[\"%s\"]=\"%s\"", it->first.c_str(), it->second.c_str());
        //     }
        //     const std::map<std::string, std::string>& headers = cntl->HttpHeaders();
        //     for (it = headers.begin(); it != headers.end(); ++it) {
        //         SLOG(INFO, "Header[\"%s\"]=\"%s\"", it->first.c_str(), it->second.c_str());
        //     }
        // }
        std::cout << request->message() << std::endl;
        response->set_message("echo message: " + request->message());
        done->Run();
    }
};

int main() {
    // server_test1("127.0.0.1", "5678");

    // Define an rpc server.
    yarpc::ServerOptions options;
    yarpc::server rpc_server(options);

    rpc_server.start("127.0.0.1", "5678");
    
    std::shared_ptr<example::EchoService> echo_service(new EchoServiceImpl);
    rpc_server.RegisterService(echo_service);
    // if (!rpc_server.RegisterService(echo_service)) {
    //     SLOG(ERROR, "export service failed");
    //     return EXIT_FAILURE;
    // }

    rpc_server.wait();
}
