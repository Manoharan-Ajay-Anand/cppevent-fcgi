#ifndef CPPEVENT_FCGI_STREAM_HPP
#define CPPEVENT_FCGI_STREAM_HPP

#include <coroutine>
#include <optional>

#include <cppevent_base/task.hpp>

namespace cppevent {

class socket;

struct stream_update_awaiter {
    std::optional<std::coroutine_handle<>>& m_producer;
    std::optional<std::coroutine_handle<>>& m_consumer;
    bool m_ended;

    bool await_ready() { return !m_consumer.has_value() && m_ended; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) {
        m_producer = handle;
        auto res_handle = m_consumer.value_or(std::noop_coroutine());
        m_consumer.reset();
        return res_handle;
    }

    void await_resume() {}
};

struct stream_read_awaiter {
    std::optional<std::coroutine_handle<>>& m_producer;
    std::optional<std::coroutine_handle<>>& m_consumer;

    bool await_ready() { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) {
        m_consumer = handle;
        auto res_handle = m_producer.value_or(std::noop_coroutine());
        m_producer.reset();
        return res_handle;
    }

    void await_resume() {}
};

class stream {
private:
    std::optional<std::coroutine_handle<>>& m_producer;
    socket& m_conn;
    std::optional<std::coroutine_handle<>> m_consumer;
    long m_remaining;
    bool m_ended;

public:
    stream(std::optional<std::coroutine_handle<>>& producer, socket& conn);

    awaitable_task<long> read(void* dest, long size);
    stream_update_awaiter update(long remaining);
};

}

#endif
