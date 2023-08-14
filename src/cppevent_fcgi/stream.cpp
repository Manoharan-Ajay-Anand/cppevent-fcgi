#include "stream.hpp"

#include <cppevent_base/event_loop.hpp>

#include <cppevent_net/socket.hpp>

#include <algorithm>
#include <cstddef>

cppevent::stream::stream(socket& conn, event_loop& loop): m_conn(conn), m_loop(loop) {
    m_remaining = 0;
    m_ended = false;
}

cppevent::stream_readable_awaiter cppevent::stream::can_read() {
    return { m_producer, m_consumer, m_loop, m_remaining, m_ended };
}

cppevent::awaitable_task<long> cppevent::stream::read(void* dest, long size) {
    std::byte* dest_ptr = static_cast<std::byte*>(dest);
    long total = 0;
    while (size > 0 && (co_await can_read())) {
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
