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
        response->set_message("echo message: " + request->message());

        // 处理完成之后调用done->Run()，触发回调
        done->Run();
    }
};

int main() {
    yarpc::ServerOptions options;
    options.uptime = 10;
    yarpc::Server rpc_server(options);

    rpc_server.Start("127.0.0.1", "5678");
    
    std::shared_ptr<example::EchoService> echo_service(new EchoServiceImpl);
    rpc_server.RegisterService(echo_service);

    rpc_server.Wait();
}
