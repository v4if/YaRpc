#include <memory>
#include <thread>
#include <iostream>
#include <unistd.h>
// valgrind ./shared_ptr_double_free_test

namespace async_callback_without_ownship {
class sft {
public:
    sft(int m) : m_(m) {}
    std::unique_ptr<std::thread> async_echo() {
        std::unique_ptr<std::thread> th (new std::thread(
            [this]() {
                sleep(3);
                std::cout << "private m_: " << m_ << "\n";
            }
        ));
        return std::move(th);
    }
private:
    int m_;
};

void test1_async_callback_without_ownship() {
    sft test(8);
    sft* T = new sft(5);
    std::unique_ptr<std::thread> th = T->async_echo();

    delete T;
    th->join();
}
} // namespace

namespace async_callback_with_shared_from_this {
class sft : public std::enable_shared_from_this<sft> {
public:
    sft(int m) : m_(m) {}
    std::unique_ptr<std::thread> async_echo() {
        std::unique_ptr<std::thread> th (new std::thread(
            [this, self = shared_from_this()]() {
                sleep(3);
                std::cout << "private m_: " << m_ << "\n";
            }
        ));
        return std::move(th);
    }
private:
    int m_;
};

void test2_async_callback_with_shared_from_this() {
    std::shared_ptr<sft> T(new sft(5));
    std::unique_ptr<std::thread> th = T->async_echo();

    T.reset();
    th->join();
}
} // namespace

void shared_ptr_copy_test(std::shared_ptr<int> p) {
    std::cout << p.use_count() << std::endl;
}

int main() {
    int* p = new int(1);
    std::shared_ptr<int> s1(p);
    // std::shared_ptr<int> s2(p);

    std::weak_ptr<int> w(s1);
    std::cout << s1.use_count() << std::endl;
    
    std::shared_ptr<int> s3 = w.lock();
    std::cout << s1.use_count() << std::endl;

    shared_ptr_copy_test(s1);

    async_callback_without_ownship::test1_async_callback_without_ownship();
    // async_callback_with_shared_from_this::test2_async_callback_with_shared_from_this();
}
    