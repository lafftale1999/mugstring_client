#ifndef LTALE_HTTP_PARSER_HPP_
#define LTALE_HTTP_PARSER_HPP_

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <sstream>
#include <initializer_list>

namespace networking::http {

using http_headers = std::unordered_map<std::string, std::string>; 

enum class METHOD {
    GET,
    POST,
    PUT,
    DELETE
};

static inline const std::unordered_map<std::string_view, METHOD> methodMap = {
    {"GET",     METHOD::GET},
    {"POST",    METHOD::POST},
    {"PUT",     METHOD::PUT},
    {"DELETE",  METHOD::DELETE}
};

static inline const char* toString(const METHOD& m) {
    switch (m) {
        case METHOD::GET:       return "GET";
        case METHOD::POST:      return "POST";
        case METHOD::PUT:       return "PUT";
        case METHOD::DELETE:    return "DELETE";
        default: throw std::runtime_error("Invalid method");
    }
}

class Request {
public:
    Request() = default;
    Request(const std::vector<char>& rawRequest);
    Request(
        METHOD method, std::string path, std::string host,
        std::initializer_list<std::pair<std::string, std::string>> headers,
        std::string body
    );

    std::string getRequest();

    METHOD getMethod() const;
    const std::string& getPath() const;
    const std::string& getVersion() const;
    const http_headers& getHeaders() const;
    const std::string& getBody() const;
    const std::string& getRawRequest() const;

    void setMethod(const std::string& method);
    void setMethod(const METHOD method);
    void setPath(std::string path);
    void setVersion(std::string version);
    void setHeader(std::string key, std::string value);
    void addHeaders(std::initializer_list<std::pair<std::string, std::string>> headers);
    void setBody(const std::string& body);

private:
    METHOD              method_;
    std::string         path_;
    std::string         version_;
    http_headers        headers_;
    std::string         body_;

    std::string         rawRequest_;
    
    void parseRequest();
    void parseStartLine(std::string& line);
    void parseHeader(std::string& header);
};

class Response {
    void setMethod(const METHOD m);
};
}

#endif