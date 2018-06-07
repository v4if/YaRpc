#ifndef YARPC_NONCOPYABLE_HPP_
#define YARPC_NONCOPYABLE_HPP_

struct noncopyable {
protected:
    noncopyable() {}
    ~noncopyable() {}
private:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
    noncopyable(noncopyable&&) = delete;
    noncopyable& operator=(noncopyable&&) = delete;
};

#endif
