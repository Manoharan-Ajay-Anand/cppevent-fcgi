#ifndef CPPEVENT_FCGI_FCGI_HANDLER_HPP
#define CPPEVENT_FCGI_FCGI_HANDLER_HPP

#include "types.hpp"

#include <cppevent_base/task.hpp>

namespace cppevent {

class stream;

class output;

class fcgi_handler {
public:
    task handle_request(stream& s_params, stream& s_stdin,
                        output& o_stdout, output& o_endreq,
                        output_queue& o_queue, bool close_conn);
};

}

#endif
