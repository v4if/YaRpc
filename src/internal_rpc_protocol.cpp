#include "internal_rpc_protocol.hpp"

namespace yarpc {

namespace internal_rpc_protocol {
    /**
     * |    uint32    |    uint32    |    data    |    data    |   uint32    |
     *     meta_size    request_size    meta_data  request_data  message_size
    */

    // Client
    // 序列化request，并添加meta
    void serialize_and_packed_request(std::string* buf_stream, const gMethodDescriptor *method, gController* controller, const gMessage* request) {

        Controller* cntl = static_cast<Controller*>(controller);

        RequestMeta request_meta;
        request_meta.set_sequence_id(cntl->next_sequence_id());
        request_meta.set_service_name(method->service()->full_name());
        request_meta.set_method_name(method->name());

        uint32 meta_size = boost::asio::detail::socket_ops::host_to_network_long(request_meta.ByteSize());
        uint32 request_size = boost::asio::detail::socket_ops::host_to_network_long(request->ByteSize());

        buf_stream->append(reinterpret_cast<char*>(&meta_size), sizeof(meta_size));
        buf_stream->append(reinterpret_cast<char*>(&request_size), sizeof(request_size));

        buf_stream->append(request_meta.SerializeAsString());
        buf_stream->append(request->SerializeAsString());
        std::cout << *buf_stream << std::endl;

        uint32 message_size = buf_stream->size() - sizeof(meta_size) - sizeof(request_size);       // message_size = meta_size + request_size;
        uint32 trans_bytes = request_meta.ByteSize() + request->ByteSize();
        if (message_size != trans_bytes) {
            return cntl->SetFailed("trans_bytes is not equal message_size");
        }

        uint32 netorder_message_size = boost::asio::detail::socket_ops::host_to_network_long(message_size);
        buf_stream->append(reinterpret_cast<char*>(&netorder_message_size), sizeof(netorder_message_size));
    }

    // server
    void process_and_unpacked_request(server* serv, boost::asio::streambuf& read_buff) {
        const std::size_t min_complete_size = 3 * sizeof(uint32);
        while (read_buff.size() >= min_complete_size) {
            boost::asio::streambuf::const_buffers_type const_buff = read_buff.data();
            std::string data(boost::asio::buffers_begin(const_buff), boost::asio::buffers_end(const_buff));

            uint32 meta_size;
            std::copy(data.begin(), data.begin() + sizeof(meta_size), reinterpret_cast<char*>(&meta_size));
            uint32 request_size;
            std::copy(data.begin() + sizeof(meta_size), data.begin() + sizeof(meta_size) + sizeof(request_size), reinterpret_cast<char*>(&request_size));

            meta_size = boost::asio::detail::socket_ops::network_to_host_long(meta_size);
            request_size = boost::asio::detail::socket_ops::network_to_host_long(request_size);

            uint32 expected_msg_size;
            uint32 next_index = sizeof(meta_size) + sizeof(request_size) + meta_size + request_size;
            if (read_buff.size() >= next_index + sizeof(expected_msg_size)) {
                // check message_size for complete
                std::copy(data.begin() + next_index, data.begin() + next_index + sizeof(expected_msg_size), reinterpret_cast<char*>(&expected_msg_size));
                expected_msg_size = boost::asio::detail::socket_ops::network_to_host_long(expected_msg_size);

                assert(expected_msg_size == meta_size + request_size);

                RequestMeta request_meta;
                request_meta.ParseFromArray(data.c_str() + sizeof(meta_size) + sizeof(request_size), meta_size);

                // gMessage request;
                // request.ParseFromArray(data.c_str() + sizeof(meta_size) + sizeof(request_size) + meta_size, request_size);
                
                std::cout << request_meta.sequence_id() <<  "\t" << request_meta.service_name() << "\t" << request_meta.method_name() << std::endl;
                read_buff.consume(min_complete_size + expected_msg_size);
            } else {
                // message not complete
                break;
            }
        }
    }

} // namespace internal_rpc_protocol

} // namespace yarpc
