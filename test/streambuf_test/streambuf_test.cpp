#include <boost/asio.hpp>
#include <iostream>

size_t fake_read(boost::asio::streambuf::mutable_buffers_type buff) {
    size_t size_ = boost::asio::buffer_size(buff);

    srand(time(NULL));
    size_t max_size_ = rand() % size_;
    std::cout << "prepare out sequence size_ : " << size_ 
        << "\t random read size_ : " << max_size_ << std::endl;
    char* p = boost::asio::buffer_cast<char*>(buff);
    for (size_t i = 0;i < max_size_;i++) {
        p[i] = (i % 10) + 0x30;
    }

    return max_size_;
}

void stream_buff_test() {
    boost::asio::streambuf sb;
    auto prev = sb.data();
    std::ostream os(&sb);
    os << "Hello world\n";
    auto next = sb.data();

    // true
    std::cout << (boost::asio::buffers_begin(prev) 
        == boost::asio::buffers_begin(next)) << std::endl; 
    // false 说明输出序列的写指针pptr向后移动了
    std::cout << (boost::asio::buffers_end(prev) 
        == boost::asio::buffers_end(next)) << std::endl;

    std::cout << sb.size() << std::endl;
    // 移动输入序列读指针gptr，并调整尾指针egptr为pptr
    sb.consume(sb.size());
    std::cout << sb.size() << std::endl;

    std::cout << sb.max_size() << std::endl;

    /*
    主要调用reverse进行内存处理
    if epptr - pptr空间够用，则直接返回
    else {
        将未读取的输入序列的内容移动到缓冲区头部
        buffer_.resize 底层存储申请内存
        setg 调整输入序列指针
        setp 调整输出序列指针
    }
    */
    boost::asio::streambuf::mutable_buffers_type out_bufs = sb.prepare(512);
    std::cout << sb.size() << std::endl;
    size_t bytes_transferred = fake_read(out_bufs);
    // 移动输出序列写指针
    sb.commit(bytes_transferred);
    std::cout << sb.size() << std::endl;

    boost::asio::streambuf::const_buffers_type in_bufs = sb.data();
    std::string data(boost::asio::buffers_begin(in_bufs), 
        boost::asio::buffers_begin(in_bufs) + bytes_transferred);
    std::cout << data << std::endl;

    sb.consume(bytes_transferred);
    std::cout << sb.size() << std::endl;
}

int main() {
    stream_buff_test();
}
