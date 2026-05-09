#ifndef LTALE_HTTP_SERVER_ROUTER_HPP_
#define LTALE_HTTP_SERVER_ROUTER_HPP_

#include "http_parser.hpp"

#include <unordered_map>

namespace networking::http {

class HttpRouter {
public:
    HttpRouter();

    void scanResources(const std::string& directory);
    Response handleRequest(const Request& request);

private:
    std::unordered_map<std::string, std::string> availableResources;

    Response routeGetRequest(const Request& request);

    uint8_t fetchContent(const std::string& path, std::string& contentOut);
    std::string parseContentType(const std::string& fName);
};
} // end of namespace

#endif