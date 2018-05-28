#ifndef noncopyable_hpp_
#define noncopyable_hpp_

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
