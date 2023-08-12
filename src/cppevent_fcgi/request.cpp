#include "request.hpp"

#include "record.hpp"

cppevent::request::request(int id, bool close_conn, socket& conn): m_id(id),
                                                                   m_close_conn(close_conn),
                                                                   m_params(m_producer, conn),
                                                                   m_stdin(m_producer, conn),
                                                                   m_data(m_producer, conn) {
}

cppevent::stream* cppevent::request::get_stream(int type) {
    switch (type) {
        case FCGI_PARAMS:
            return &m_params;
        case FCGI_STDIN:
            return &m_stdin;
        case FCGI_DATA:
            return &m_data;
    }
    return nullptr;
}

cppevent::stream_update_awaiter cppevent::request::update(int type, long remaining) {
    return get_stream(type)->update(remaining);
}
