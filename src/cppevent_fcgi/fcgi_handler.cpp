#include "fcgi_handler.hpp"

#include "router.hpp"
#include "stream.hpp"
#include "output.hpp"
#include "context.hpp"

#include <cppevent_base/util.hpp>

#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <memory>

cppevent::fcgi_handler::fcgi_handler(router& r): m_router(r) {    
}

cppevent::awaitable_task<std::pair<long, long>> get_lengths(cppevent::stream& s) {
    long result[2];
    uint8_t data[4];
    for (int i = 0; i < 2; ++i) {
        co_await s.read(data, 1);
        if ((data[0] >> 7) == 0) {
            result[i] = data[0];
            continue;
        }
        data[0] = data[0] & 0x7f;
        co_await s.read(data + 1, 3);
        result[i] = cppevent::read_u32_be(data);
    }
    co_return { result[0], result[1] };
}

cppevent::awaitable_task<void> cppevent::fcgi_handler::handle_request(stream& s_params,
                                                                      stream& s_stdin,
                                                                      output& o_stdout,
                                                                      output& o_endreq,
                                                                      output_queue& o_queue,
                                                                      bool close_conn) {
    std::unordered_map<std::string_view, std::string_view> header_map;
    std::vector<std::unique_ptr<char[]>> header_buf;
    while ((co_await s_params.can_read())) {
        auto [name_l, val_l] = co_await get_lengths(s_params);
        long total_l = name_l + val_l;
        if (val_l == 0) {
            co_await s_params.skip(total_l);
            continue;
        }
        char* data = new char[total_l];
        co_await s_params.read(data, total_l);
        std::string_view name = { data, static_cast<std::size_t>(name_l) };
        std::string_view value = { data + name_l, static_cast<std::size_t>(val_l) };
        header_map[name] = value;
        header_buf.push_back(std::unique_ptr<char[]>{ data });
    }
    context cont { std::move(header_map) };
    co_await m_router.process(cont, s_stdin, o_stdout);
    co_await s_stdin.skip(LONG_MAX);
    co_await o_stdout.end();
    char data[8] = {};
    co_await o_endreq.write(data, 8);
    if (close_conn) {
        o_queue.push({ true, {}, nullptr, 0, { 0, nullptr } });
    }
}
