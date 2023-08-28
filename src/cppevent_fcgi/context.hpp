#ifndef CPPEVENT_FCGI_CONTEXT_HPP
#define CPPEVENT_FCGI_CONTEXT_HPP

#include "types.hpp"

#include <unordered_map>
#include <string_view>
#include <optional>

namespace cppevent {

class context {
private:
    std::unordered_map<std::string_view, std::string_view> m_params;
    std::unordered_map<std::string_view, std::string_view> m_path_segments;

    std::string_view m_path;

    std::string_view m_content_type;
    long m_content_length;

    REQUEST_METHOD m_req_method;
public:
    context(const std::unordered_map<std::string_view, std::string_view>& params);

    std::optional<std::string_view> get_param(std::string_view key) const;
    
    std::optional<std::string_view> get_path_segment(std::string_view name) const;
    void set_path_segment(std::string_view name, std::string_view val);

    std::string_view get_path() const;

    std::string_view get_content_type() const;
    long get_content_len() const;

    REQUEST_METHOD get_req_method() const;
};

}

#endif
