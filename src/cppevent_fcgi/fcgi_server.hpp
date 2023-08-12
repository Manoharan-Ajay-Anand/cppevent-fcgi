#ifndef CPPEVENT_FCGI_FCGI_SERVER_HPP
#define CPPEVENT_FCGI_FCGI_SERVER_HPP

#include "types.hpp"

#include <cppevent_net/server.hpp>

#include <cppevent_base/task.hpp>
#include <cppevent_base/async_queue.hpp>

#include <string>
#include <memory>

namespace cppevent {

class request;

class event_loop;

class fcgi_server : public connection_handler {
private:
    event_loop& m_loop;
    server m_server;
    std::unordered_map<int, request> m_requests;

    awaitable_task<void> write_res(socket& sock, async_queue<output_cmd>& out_queue);
    awaitable_task<void> read_req(socket& sock, async_queue<output_cmd>& out_queue);
public:
    fcgi_server(const char* name, const char* service, event_loop& loop);
    fcgi_server(const std::string& name, const std::string& service, event_loop& loop);

    task on_connection(std::unique_ptr<socket> sock);
};

}

#endif
