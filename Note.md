cmake
unique_ptr shared_ptr weak_ptr
git submodule

扩展性、维护性、健壮性
强侵入设计

boost

两个线程操作指向同一个资源的不同weak_ptr是安全的，但操作同一个不安全，比如在同一个weak_ptr线程A调用swap, 线程B把这个weak_ptr reset了，后果，嘿嘿。

_shared_ptr直接包含的裸指针是为了实现一般指针的->,*等操作，通过__shared_count间接包含的指针是为了管理对象的生命周期，回收相关资源。
换句话说，__shared_count内部的use_count主要用来标记被管理对象的生命周期，weak_count主要用来标记管理对象的生命周期。
_Tp*             _M_ptr;         // Contained pointer.  
__shared_count<_Lp>  _M_refcount;    // Reference counter.  


在许多语言里, 垃圾收集并不是编译器实现的, 而是由语言附带的运行时环境实现的, 编译器为运行时提供了附加的信息. 这就导致了语言和运行时的强耦合. 让人无法分清语言的特性和运行时的特性.

win的Linux子系统下面不支持netstat命令，可以在cygwin下面查看
在Cygwin终端上右键-->Options…-->Text-->修改Locale 为 zh_CN，Character Set 为 GBK，问题便得到解决。


task_cleanup on_exit = { this, &lock, &this_thread };
(void)on_exit;

enable_shared_from_this
需求: 在类的内部需要自身的shared_ptr 而不是this裸指针
场景:  在类中发起一个异步操作, callback回来要保证发起操作的对象仍然有效.


gperftools

EchoService  server
EchoService_Stub client


NEVER start your second async_write before the first has completed 
async_write 串流
asio异步发送复杂的地方在于: 不能连续调用异步发送接口async_write，因为async_write内部是不断调用async_write_some，直到所有的数据发送完成为止。由于async_write调用之后就直接返回了，如果第一次调用async_write发送一个较大的包时，马上又再调用async_write发送一个很小的包时，有可能这时第一次的async_write还在循环调用async_write_some发送，而第二次的async_write要发送的数据很小，一下子就发出去了，这使得第一次发送的数据和第二次发送的数据交织在一起了，导致发送乱序的问题。解决这个问题的方法就是在第一次发送完成之后再发送第二次的数据。具体的做法是用一个发送缓冲区，在异步发送完成之后从缓冲区再取下一个数据包发送。
