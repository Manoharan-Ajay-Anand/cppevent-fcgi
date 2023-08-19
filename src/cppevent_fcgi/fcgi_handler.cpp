#include "fcgi_handler.hpp"

#include "stream.hpp"
#include "output.hpp"

#include <cppevent_base/util.hpp>

#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <iostream>

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

struct header {
    std::vector<char>& header_buf;
    long name_offset;
    long name_len;
    long val_offset;
    long val_len;

    std::string_view get_name() const {
        return { header_buf.data() + name_offset, static_cast<std::size_t>(name_len) };
    }

    std::string_view get_value() const {
        return { header_buf.data() + val_offset, static_cast<std::size_t>(val_len) };
    }
};

cppevent::awaitable_task<void> cppevent::fcgi_handler::handle_request(stream& s_params,
                                                                      stream& s_stdin,
                                                                      output& o_stdout,
                                                                      output& o_endreq,
                                                                      output_queue& o_queue,
                                                                      bool close_conn) {
    std::unordered_map<std::string_view, std::string_view> header_map;
    std::vector<char> header_buf;
    std::vector<header> headers;
    header_buf.reserve(1024);
    long h_offset = 0;
    while ((co_await s_params.can_read())) {
        auto [name_l, val_l] = co_await get_lengths(s_params);
        long total_l = name_l + val_l;
        header_buf.resize(header_buf.size() + total_l);
        co_await s_params.read(header_buf.data() + h_offset, total_l);
        headers.push_back({ header_buf, h_offset, name_l, h_offset + name_l, val_l });
        h_offset += total_l;
    }
    for (const auto& header : headers) {
        header_map[header.get_name()] = header.get_value();
    }
    for (auto& p : header_map) {
        std::cout << p.first << ": " << p.second << std::endl;
    }
    std::cout << std::endl;
    co_await o_stdout.write("content-length: 5\n");
    co_await o_stdout.write("content-type: text/plain\n\nhello");
    co_await o_stdout.end();
    char data[8] = {};
    co_await o_endreq.write(data, 8);
    if (close_conn) {
        o_queue.push({ true, {}, nullptr, 0, { 0, nullptr } });
    }
}
