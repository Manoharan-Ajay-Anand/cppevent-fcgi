add_library(cppevent-fcgi STATIC
    src/cppevent_fcgi/fcgi_server.cpp
    src/cppevent_fcgi/record.cpp
    src/cppevent_fcgi/request.cpp
    src/cppevent_fcgi/stream.cpp
    src/cppevent_fcgi/stream_awaiters.cpp
    src/cppevent_fcgi/output.cpp
    src/cppevent_fcgi/fcgi_handler.cpp
)

target_include_directories(cppevent-fcgi PUBLIC src)

target_link_libraries(cppevent-fcgi PUBLIC cppevent-net)
