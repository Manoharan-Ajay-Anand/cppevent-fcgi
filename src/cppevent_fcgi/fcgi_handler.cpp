#include "fcgi_handler.hpp"

#include "stream.hpp"
#include "output.hpp"

#include <cppevent_base/util.hpp>

#include <cstdint>
#include <string>
#include <iostream>

cppevent::awaitable_task<long> get_length(cppevent::stream& s) {
    uint8_t data[4];
    co_await s.read(data, 1);
    if ((data[0] >> 7) == 0) {
        co_return { data[0] };
    }
    data[0] = data[0] & 0x7f;
    co_await s.read(data + 1, 3);
    co_return { cppevent::read_u32_be(data) };
}

cppevent::awaitable_task<void> cppevent::fcgi_handler::handle_request(stream& s_params,
                                                                      stream& s_stdin,
                                                                      output& o_stdout,
                                                                      output& o_endreq,
                                                                      output_queue& o_queue,
                                                                      bool close_conn) {
    while ((co_await s_params.can_read())) {
        long name_l = co_await get_length(s_params);
        long val_l = co_await get_length(s_params);
        std::string name(name_l, '\0');
        std::string val(val_l, '\0');
        co_await s_params.read(name.data(), name_l);
        co_await s_params.read(val.data(), val_l);
        std::cout << name << ": " << val << std::endl;  
    }
    co_await o_stdout.end();
    char data[8] = {};
    co_await o_endreq.write(data, 8);
    if (close_conn) {
        o_queue.push({ true, {}, nullptr, 0, { 0, nullptr } });
    }
}
