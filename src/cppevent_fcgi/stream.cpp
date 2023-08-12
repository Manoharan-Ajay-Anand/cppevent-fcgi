#include "stream.hpp"

#include <cppevent_net/socket.hpp>

#include <algorithm>
#include <cstddef>

cppevent::stream::stream(std::optional<std::coroutine_handle<>>& producer,
                         socket& conn): m_producer(producer),
                                        m_conn(conn) {
    m_remaining = 0;
    m_ended = false;
}

cppevent::awaitable_task<long> cppevent::stream::read(void* dest, long size) {
    std::byte* dest_ptr = static_cast<std::byte*>(dest);
    long total = 0;
    while (size > 0 && !m_ended) {
        if (m_remaining == 0) {
            co_await stream_read_awaiter { m_producer, m_consumer };
            continue;
        }
        long transferred = co_await m_conn.read(dest_ptr, std::min(size, m_remaining), true);
        dest_ptr += transferred;
        total += transferred;
        size -= transferred;
        m_remaining -= transferred;
    }
    co_return total;
}

cppevent::stream_update_awaiter cppevent::stream::update(long remaining) {
    m_ended = remaining == 0;
    m_remaining = remaining;
    return { m_producer, m_consumer, m_ended };
}
