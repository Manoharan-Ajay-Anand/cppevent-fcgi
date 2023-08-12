#ifndef CPPEVENT_FCGI_REQUEST_HPP
#define CPPEVENT_FCGI_REQUEST_HPP

#include "stream.hpp"

#include <coroutine>
#include <optional>

namespace cppevent {

class socket;

class request {
private:
    const int m_id;
    const bool m_close_conn;
    std::optional<std::coroutine_handle<>> m_producer;

    stream m_params;
    stream m_stdin;
    stream m_data;

    stream* get_stream(int type);

public:
    request(int id, bool close_conn, socket& conn);

    stream_update_awaiter update(int type, long remaining);
};

}

#endif
