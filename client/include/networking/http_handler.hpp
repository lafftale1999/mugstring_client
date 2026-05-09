#ifndef LTALE_HTTP_HANDLER_HPP_
#define LTALE_HTTP_HANDLER_HPP_

#include "http_parser.hpp"

namespace networking::http {

class HttpHandler {
public:
    static Response buildResponse(const RESPONSE_CODE rCode);
    static Response handleRequest(const Request& request);
};

} // end of namespace

#endif