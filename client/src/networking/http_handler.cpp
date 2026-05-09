#include "../../include/networking/http_handler.hpp"

namespace networking::http {
    static void additionalHeaders(Response& r) {

    }

    Response HttpHandler::buildResponse(const RESPONSE_CODE rCode) {
        Response r;

        r.setResponseCode(rCode);
        additionalHeaders(r);

        return r;
    }

    Response HttpHandler::handleRequest(const Request& request) {
        
    }
} 