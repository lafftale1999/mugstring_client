#include "../../include/networking/http_parser.hpp"

#include <exception>
#include <iterator>
#include <string>
#include <algorithm>
#include <regex>

namespace networking::http {
static void removeSpecialChar(std::string& s) {
    auto it = std::remove_if(s.begin(), s.end(), [](const auto& c){
        return c == '\n' || c == '\r';
    });

    if (it != s.end()) {
        s.erase(it, s.end());
    }
}

/**
 * ------------------------ REQUEST ---------------------------------------
 */
Request::Request(const std::vector<char>& rawRequest) 
: rawRequest_(rawRequest.begin(), rawRequest.end()) {
    if (rawRequest_.empty()) {
        throw std::runtime_error("The request is empty");
    }

    parseRequest();
}

METHOD Request::getMethod() const {
    return method_;
}

const std::string& Request::getPath() const {
    return path_;
}

const std::string& Request::getVersion() const {
    return version_;
}

const http_headers& Request::getHeaders() const {
    return headers_;
}

const std::string& Request::getBody() const {
    return body_;
}

const std::string& Request::getRawRequest() const {
    return rawRequest_;
}

void Request::setMethod(const std::string& method) {
    try {
        method_ = methodMap.at(method);
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to set http method");
    }
}

void Request::setPath(std::string path) {
    if (path.empty() || path.at(0) != '/') {
        throw std::runtime_error("Path is invalid");
    }

    path_ = std::move(path);
}

void Request::setVersion(std::string version) {
    static const auto rVersion = std::regex(R"(\d\.\d)");
    auto match = std::regex_match(version, rVersion);

    if (!match) {
        throw std::runtime_error("Version is invalid");
    }

    version_ = std::move(version);
}

void Request::setHeader(std::string key, std::string value) {
    headers_[std::move(key)] = std::move(value);
}

void Request::setBody(const std::string& body) {
    body_ = std::move(body); 
}

void Request::parseRequest() {
    std::stringstream ss(rawRequest_);

    std::string buf;
    std::getline(ss, buf, '\n');

    parseStartLine(buf);

    while(std::getline(ss, buf, '\n') && !buf.empty() && buf != "\r") {
        parseHeader(buf);
    }
    
    std::getline(ss, buf);
    setBody(buf);
}

void Request::parseStartLine(std::string& line) {
    removeSpecialChar(line);
    std::stringstream ss(line);
    std::string temp;

    std::getline(ss, temp, ' ');
    setMethod(temp);

    std::getline(ss, temp, ' ');
    setPath(temp);

    std::getline(ss, temp, ' ');
    auto pos = temp.find('/');
    setVersion(std::string(temp.begin() + pos + 1, temp.end()));
}

void Request::parseHeader(std::string& header) {
    removeSpecialChar(header);
    
    auto sep = header.find(':');
    std::string key = std::string(header.substr(0, sep));
    std::string val = std::string(header.substr(sep + 2));
    
    setHeader(key, val);
}
}
