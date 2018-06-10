#include "internal_rpc_protocol.hpp"
#include "controller.hpp"
#include "server.hpp"
#include "message_op.hpp"

// #include "echo.pb.h"

namespace yarpc {

namespace internal_rpc_protocol {
    /**
     * |    uint32    |    uint32    |    data    |    data    |   uint32    |
     *     meta_size    request_size    meta_data  request_data  message_size
    */

    // client
    uint64 serialize_and_packed_request(std::string* buf_stream, 
        const gMethodDescriptor *method, 
        gController* controller, 
        const gMessage* request) {

        Controller* cntl = static_cast<Controller*>(controller);
        uint64 sequence_id = cntl->next_sequence_id();

        RequestMeta request_meta;
        request_meta.set_sequence_id(sequence_id);
        request_meta.set_service_name(method->service()->full_name());
        request_meta.set_method_name(method->name());

        uint32 meta_size = 
            boost::asio::detail::socket_ops::host_to_network_long(request_meta.ByteSize());
        uint32 request_size = 
            boost::asio::detail::socket_ops::host_to_network_long(request->ByteSize());

        buf_stream->append(reinterpret_cast<char*>(&meta_size), sizeof(meta_size));
        buf_stream->append(reinterpret_cast<char*>(&request_size), sizeof(request_size));

        buf_stream->append(request_meta.SerializeAsString());
        buf_stream->append(request->SerializeAsString());
        // std::cout << *buf_stream << std::endl;

        // message_size = meta_size + request_size;
        uint32 message_size = buf_stream->size() - sizeof(meta_size) - sizeof(request_size);       
        uint32 trans_bytes = request_meta.ByteSize() + request->ByteSize();
        if (message_size != trans_bytes) {
            cntl->SetFailed("trans_bytes is not equal message_size");
            // 返回值不可用，依靠cntl判断是否正确
            return -1;
        }

        uint32 netorder_message_size = 
            boost::asio::detail::socket_ops::host_to_network_long(message_size);
        buf_stream->append(reinterpret_cast<char*>(&netorder_message_size), sizeof(netorder_message_size));

        return sequence_id;
    }

    void process_and_unpacked_response(boost::asio::streambuf& read_buff,
        google::protobuf::RpcController* controller) {

        Controller* cntl = static_cast<Controller*>(controller);

        const std::size_t min_complete_size = 3 * sizeof(uint32);
        while (read_buff.size() >= min_complete_size) {
            boost::asio::streambuf::const_buffers_type const_buff = read_buff.data();
            std::string data(boost::asio::buffers_begin(const_buff), boost::asio::buffers_end(const_buff));

            uint32 meta_size;
            std::copy(data.begin(), data.begin() + sizeof(meta_size), reinterpret_cast<char*>(&meta_size));
            uint32 response_size;
            std::copy(data.begin() + sizeof(meta_size), 
                data.begin() + sizeof(meta_size) + sizeof(response_size), 
                reinterpret_cast<char*>(&response_size));

            meta_size = boost::asio::detail::socket_ops::network_to_host_long(meta_size);
            response_size = boost::asio::detail::socket_ops::network_to_host_long(response_size);

            uint32 expected_msg_size;
            uint32 next_index = sizeof(meta_size) + sizeof(response_size) + meta_size + response_size;
            if (read_buff.size() >= next_index + sizeof(expected_msg_size)) {
                // check message_size for complete
                std::copy(data.begin() + next_index, 
                    data.begin() + next_index + sizeof(expected_msg_size), 
                    reinterpret_cast<char*>(&expected_msg_size));
                expected_msg_size = boost::asio::detail::socket_ops::network_to_host_long(expected_msg_size);

                assert(expected_msg_size == meta_size + response_size);

                ResponseMeta response_meta;
                response_meta.ParseFromArray(data.c_str() + sizeof(meta_size) + sizeof(response_size), meta_size);

                std::shared_ptr<message_op> msg_op = 
                    cntl->find_pending_call_by_sequenceid(response_meta.sequence_id());
                msg_op->response->ParseFromArray(data.c_str() + sizeof(meta_size) + sizeof(response_size) + meta_size, 
                    response_size);

                msg_op->notify();
                msg_op->done();
                
                cntl->remove_pending_call(response_meta.sequence_id());

                read_buff.consume(min_complete_size + expected_msg_size);
            } else {
                // message not complete
                break;
            }
        }
    }

    /**
     * |    uint32    |    uint32    |    data    |    data    |   uint32    | 
     *     meta_size   response_size res_meta_data response_data  message_size
    */
    // server
    void serialize_and_packed_response(std::string* buf_stream, 
        gController* controller, 
        const gMessage* response) {

        Controller* cntl = static_cast<Controller*>(controller);

        ResponseMeta response_meta;
        response_meta.set_sequence_id(cntl->get_reply_sequence_id());
        response_meta.set_error_code(0);
        response_meta.set_error_text(std::string());

        uint32 meta_size = 
            boost::asio::detail::socket_ops::host_to_network_long(response_meta.ByteSize());
        uint32 response_size = 
            boost::asio::detail::socket_ops::host_to_network_long(response->ByteSize());

        buf_stream->append(reinterpret_cast<char*>(&meta_size), sizeof(meta_size));
        buf_stream->append(reinterpret_cast<char*>(&response_size), sizeof(response_size));

        buf_stream->append(response_meta.SerializeAsString());
        buf_stream->append(response->SerializeAsString());
        // std::cout << *buf_stream << std::endl;

        // message_size = meta_size + request_size;
        uint32 message_size = buf_stream->size() - sizeof(meta_size) - sizeof(response_size);       
        uint32 trans_bytes = response_meta.ByteSize() + response->ByteSize();
        if (message_size != trans_bytes) {
            return cntl->SetFailed("trans_bytes is not equal message_size");
        }

        uint32 netorder_message_size = 
            boost::asio::detail::socket_ops::host_to_network_long(message_size);
        buf_stream->append(reinterpret_cast<char*>(&netorder_message_size), sizeof(netorder_message_size));
    }

    void process_and_unpacked_request(server* serv, 
        std::weak_ptr<server_internal::session> session, 
        boost::asio::streambuf& read_buff) {

        const std::size_t min_complete_size = 3 * sizeof(uint32);
        while (read_buff.size() >= min_complete_size) {
            boost::asio::streambuf::const_buffers_type const_buff = read_buff.data();
            std::string data(boost::asio::buffers_begin(const_buff), boost::asio::buffers_end(const_buff));

            uint32 meta_size;
            std::copy(data.begin(), data.begin() + sizeof(meta_size), reinterpret_cast<char*>(&meta_size));
            uint32 request_size;
            std::copy(data.begin() + sizeof(meta_size), 
                data.begin() + sizeof(meta_size) + sizeof(request_size), 
                reinterpret_cast<char*>(&request_size));

            meta_size = boost::asio::detail::socket_ops::network_to_host_long(meta_size);
            request_size = boost::asio::detail::socket_ops::network_to_host_long(request_size);

            uint32 expected_msg_size;
            uint32 next_index = sizeof(meta_size) + sizeof(request_size) + meta_size + request_size;
            if (read_buff.size() >= next_index + sizeof(expected_msg_size)) {
                // check message_size for complete
                std::copy(data.begin() + next_index, 
                    data.begin() + next_index + sizeof(expected_msg_size), 
                    reinterpret_cast<char*>(&expected_msg_size));
                expected_msg_size = boost::asio::detail::socket_ops::network_to_host_long(expected_msg_size);

                assert(expected_msg_size == meta_size + request_size);

                RequestMeta request_meta;
                request_meta.ParseFromArray(data.c_str() + sizeof(meta_size) + sizeof(request_size), meta_size);

                std::unique_ptr<google::protobuf::Message> request;
                std::unique_ptr<google::protobuf::Message> response;
                std::unique_ptr<Controller> cntl(new Controller);
                std::shared_ptr<google::protobuf::Service> service = 
                    serv->find_service_by_fulllname(request_meta.service_name());
                const google::protobuf::MethodDescriptor* method = 
                    service->GetDescriptor()->FindMethodByName(request_meta.method_name());
                
                request.reset(service->GetRequestPrototype(method).New());
                request->ParseFromArray(data.c_str() + sizeof(meta_size) + sizeof(request_size) + meta_size, 
                    request_size);
                response.reset(service->GetResponsePrototype(method).New());
                
                // const example::EchoRequest* req = 
                //     reinterpret_cast<const example::EchoRequest*>(request.get());
                // std::cout << req->message() << std::endl;

                cntl->set_reply_sequence_id(request_meta.sequence_id());
                cntl->set_reply_session(session);
                
                // 销毁指针
                google::protobuf::Closure* done = 
                    google::protobuf::NewCallback<server, gController*, const gMessage*>(
                        serv, 
                        &server::send_rpc_reply, 
                        cntl.get(), 
                        response.get()
                    );
                service->CallMethod(method, cntl.release(), request.get(), response.release(), done);

                read_buff.consume(min_complete_size + expected_msg_size);
            } else {
                // message not complete
                break;
            }
        }
    }

} // namespace internal_rpc_protocol

} // namespace yarpc

