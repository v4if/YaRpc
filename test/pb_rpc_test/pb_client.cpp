#include "../../src/client.hpp"
#include "../../src/channel.hpp"
#include "../../src/controller.hpp"
#include "echo.pb.h"

int main()
{
    yarpc::ClientOptions options;
    options.uptime = 5;
    yarpc::Client clt(options);
    yarpc::Channel channel(&clt, "127.0.0.1", 5678);

    clt.Start();

    example::EchoService_Stub stub(&channel);
    example::EchoRequest request;
    request.set_message("Hello world!");
 
    example::EchoResponse response;

    yarpc::Controller controller;
 
    stub.Echo(&controller, &request, &response, NULL);

    std::cout << response.message() << std::endl;

    clt.Wait();
    return 0;
}
