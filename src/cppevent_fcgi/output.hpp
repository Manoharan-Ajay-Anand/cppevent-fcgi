#ifndef CPPEVENT_FCGI_OUTPUT_HPP
#define CPPEVENT_FCGI_OUTPUT_HPP

#include "types.hpp"

#include <cppevent_base/async_signal.hpp>
#include <cppevent_base/async_queue.hpp>
#include <cppevent_base/event_listener.hpp>

#include <string_view>

namespace cppevent {

class event_loop;

class output {
private:
    const int m_req_id;
    const int m_type;
    output_queue& m_out_queue;
    async_signal m_signal;
public:
    output(int req_id, int type, output_queue& out_queue, event_loop& loop);

    read_awaiter write(const void* src, long size);
    read_awaiter write(std::string_view s);
    read_awaiter end();
};

}

#endif
