#include "router.hpp"

#include "endpoint.hpp"
#include "output.hpp"

#include <stdexcept>

void cppevent::route_node::insert(const std::vector<std::string_view>& segments, long i,
                                  endpoint& endpoint, REQUEST_METHOD method) {
    if (i == segments.size()) {
        switch (method) {
            case REQUEST_METHOD::GET:
                m_get_endpoint = &endpoint;
                break;
            case REQUEST_METHOD::POST:
                m_post_endpoint = &endpoint;
                break;
        }
        return;
    }
    auto segment = segments[i];
    route_node* next_node = nullptr;
    if (segment.size() > 2 && segment.front() == '{' && segment.back() == '}') {
        auto variable = segment.substr(1, segment.size() - 2);
        if (!m_var_node) {
            m_variable = std::string { variable };
            m_var_node = std::make_unique<route_node>();
        } else if (variable != m_variable) {
            throw std::runtime_error("Multiple variable definition for path segment");
        }
        next_node = m_var_node.get();
    } else {
        next_node = &(m_paths[std::string { segment }]);
    }
    next_node->insert(segments, i + 1, endpoint, method);
}

cppevent::awaitable_task<void> route_not_found(cppevent::output& o_stdout) {
    co_await o_stdout.write("status: 404\ncontent-length: 0\n\n");
}

cppevent::awaitable_task<void> cppevent::route_node::process(
        const std::vector<std::string_view>& segments, long i,
        context& cont, stream& s_stdin, output& o_stdout) {
    if (i == segments.size()) {
        endpoint* e = nullptr;
        switch (cont.get_req_method()) {
            case REQUEST_METHOD::GET:
                e = m_get_endpoint;
                break;
            case REQUEST_METHOD::POST:
                e = m_post_endpoint;
                break;
        }
        if (e != nullptr) {
            return e->process(cont, s_stdin, o_stdout);
        }
    } else {
        auto segment = segments[i];
        auto it = m_paths.find(std::string { segment });
        if (it != m_paths.end()) {
            return it->second.process(segments, i + 1, cont, s_stdin, o_stdout);
        }
        if (m_var_node) {
            cont.set_path_segment(m_variable, segment);
            return m_var_node->process(segments, i + 1, cont, s_stdin, o_stdout);
        }
    }
    return route_not_found(o_stdout);
}

std::vector<std::string_view> split_path(std::string_view path) {
    std::vector<std::string_view> segments;
    long prev_slash = -1;
    for (long i = 0; i <= path.size(); ++i) {
        if (i < path.size() && path[i] != '/') {
            continue;
        }
        long segment_start = prev_slash + 1;
        const char * segment_p = path.data() + segment_start;
        long segment_size = i - segment_start;
        if (segment_size > 0) {
            segments.push_back({ segment_p, static_cast<std::size_t>(segment_size) });
        }
        prev_slash = i;
    }
    return segments;
}

void cppevent::router::get(std::string_view path, endpoint& endpoint) {
    std::vector<std::string_view> segments = split_path(path);
    m_node.insert(segments, 0, endpoint, REQUEST_METHOD::GET);
}

void cppevent::router::post(std::string_view path, endpoint& endpoint) {
    std::vector<std::string_view> segments = split_path(path);
    m_node.insert(segments, 0, endpoint, REQUEST_METHOD::POST);
}

cppevent::awaitable_task<void> cppevent::router::process(context& cont, stream& s_stdin,
                                                         output& o_stdout) {
    std::vector<std::string_view> segments = split_path(cont.get_path());
    return m_node.process(segments, 0, cont, s_stdin, o_stdout);
}
