#ifndef UTILS_HPP
#define UTILS_HPP

#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <ostream>
#include <queue>

struct record
{
    int value;
    bool flag;
    std::shared_ptr<std::string> data;

    record(int v, bool f, const std::shared_ptr<std::string> &ptr) : value(v), flag(f), data(ptr) {}
};

/*
Used for the output - the format should be in this way.
*/
std::ostream &operator<<(std::ostream &os, const record &r)
{
    os << '{' << r.value << ";" << (r.flag ? "true;" : "false;")
       << (r.data != nullptr ? (*r.data) : "NULL") << '}';
    return os;
}

struct condition
{
    virtual bool eval(const record &) const = 0;
};

struct query_condition
{
    std::list<std::shared_ptr<condition>> conditions;
    std::list<record> hits;
};

using query = std::shared_ptr<query_condition>;

template <typename T>
class Pipe
{
public:
    Pipe() = default;

    Pipe &operator=(const Pipe &) = delete;
    Pipe &operator=(const Pipe &&) = delete;

    Pipe(const Pipe &) = delete;
    Pipe(const Pipe &&) = delete;

    void push(const T &d)
    {
        std::unique_lock<std::mutex> lk(_m);
        _q.push(d);
        _cv.notify_all();
    }

    T pop()
    {
        std::unique_lock<std::mutex> lk(_m);
        if (_q.empty())
        {
            _cv.wait(lk, [=] { return !_q.empty(); });
        }

        T data = _q.front();
        _q.pop();
        return data;
    }

private:
    std::queue<T> _q;
    std::mutex _m;
    std::condition_variable _cv;
};

#endif
