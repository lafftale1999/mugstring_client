#include "../../include/networking/http_parser.hpp"
#include "../../include/networking/networking_config.hpp"

#include <exception>
#include <iterator>
#include <string>
#include <algorithm>
#include <regex>
#include <ctime>

namespace networking::http {

/**
 * ------------------------------------------------------------------------
 * ------------------------ STATIC FUNCS ----------------------------------
 * ------------------------------------------------------------------------
 */

static void removeSpecialChar(std::string& s) {
    auto it = std::remove_if(s.begin(), s.end(), [](const auto& c){
        return c == '\n' || c == '\r';
    });

    if (it != s.end()) {
        s.erase(it, s.end());
    }
}

/**
 * ------------------------------------------------------------------------
 * ------------------------ HttpMethod ------------------------------------
 * ------------------------------------------------------------------------
 */

static inline const std::unordered_map<std::string_view, METHOD> methods = {
    {"GET",     METHOD::GET},
    {"POST",    METHOD::POST},
    {"PUT",     METHOD::PUT},
    {"DELETE",  METHOD::DELETE}
};

HttpMethod::HttpMethod(METHOD m)
: eMethod_(m), sMethod_("UNSET") {
    sMethod_ = getMethodString();
}

HttpMethod::HttpMethod(std::string s)
: eMethod_(METHOD::UNSET), sMethod_(s) {
    eMethod_ = getMethodEnum();
}

METHOD HttpMethod::getMethodEnum() {
    for (auto& [s, e] : methods) {
        if (s == sMethod_) return e;
    }

    throw std::runtime_error("Unkown Method: " + sMethod_);
}

std::string HttpMethod::getMethodString() {
    for (auto& [s, e] : methods) {
        if (eMethod_ == e) return std::string(s);
    }

    throw std::runtime_error("Unable to find method string");
}

/**
 * ------------------------------------------------------------------------
 * ------------------------ RESPONSE CODE ---------------------------------
 * ------------------------------------------------------------------------
 */

static inline const std::unordered_map<std::string_view, RESPONSE_CODE> responseCodes = {
    {"OK",                      RESPONSE_CODE::OK},
    {"CREATED",                 RESPONSE_CODE::CREATED},
    {"BAD REQUEST",             RESPONSE_CODE::BAD_REQUEST},
    {"UNAUTHORIZED",            RESPONSE_CODE::UNAUTHORIZED},
    {"NOT FOUND",               RESPONSE_CODE::NOT_FOUND},
    {"CONTENT_TOO_LARGE",       RESPONSE_CODE::CONTENT_TOO_LARGE},
    {"NOT ACCEPTABLE",          RESPONSE_CODE::NOT_ACCEPTABLE},
    {"INTERNAL_SERVER_ERROR",   RESPONSE_CODE::INTERNAL_SERVER_ERROR}
};

HttpResponseCode::HttpResponseCode(RESPONSE_CODE r)
: eCode(r), sCode("UNSET") {
    sCode = getMessageString();
}

HttpResponseCode::HttpResponseCode(std::string r) 
: eCode(RESPONSE_CODE::UNSET), sCode(r) {
    eCode = getCodeEnum();
}

RESPONSE_CODE HttpResponseCode::getCodeEnum() {
    for (auto& [s, e] : responseCodes) {
        if (sCode == s) return e;
    }

    throw std::runtime_error("Unkown Response Code: " + sCode);
}

std::string HttpResponseCode::getMessageString() {
    for (auto& [s, e] : responseCodes) {
        if (eCode == e) return std::string(s);
    }

    throw std::runtime_error("Unkown Response Code for string");
}

std::string HttpResponseCode::getCodeString() {
    return std::to_string(static_cast<int>(eCode));
}

/**
 * ------------------------------------------------------------------------
 * ------------------------ HTTP BASE -------------------------------------
 * ------------------------------------------------------------------------
 */

HttpMessageBase::~HttpMessageBase() {};

const std::string& HttpMessageBase::getVersion() const {
    return version_;
}

const http_headers& HttpMessageBase::getHeaders() const {
    return headers_;
}

const std::string& HttpMessageBase::getBody() const {
    return body_;
}

void HttpMessageBase::setVersion(std::string version) {
    static const auto rVersion = std::regex(R"(\d\.\d)");
    auto match = std::regex_match(version, rVersion);

    if (!match) {
        throw std::runtime_error("Version is invalid");
    }

    version_ = std::move(version);
}

void HttpMessageBase::setHeader(std::string key, std::string value) {
    headers_[std::move(key)] = std::move(value);
}

void HttpMessageBase::addHeaders(std::initializer_list<std::pair<std::string, std::string>> headers) {
    for (const auto& h : headers) {
        setHeader(h.first, h.second);
    }
}

void HttpMessageBase::setBody(const std::string& body) {
    body_ = std::move(body); 
}


/**
 * ------------------------------------------------------------------------
 * ------------------------ REQUEST ---------------------------------------
 * ------------------------------------------------------------------------
 */
Request::Request(std::string rawRequest) 
: rawRequest_(std::move(rawRequest)) {
    if (rawRequest_.empty()) {
        throw std::runtime_error("The request is empty");
    }

    parseRequest();
}

Request::Request(
    METHOD method, std::string path, std::string host,
    std::initializer_list<std::pair<std::string, std::string>> headers,
    std::string body) : method_(method)
    {
    setPath(path);
    setVersion("1.1");
    setHeader("Host", host);
    addHeaders(headers);
    setBody(body);
}

Request::~Request() {};

std::string Request::getRequest() {
    std::stringstream ss;

    ss << method_.getMethodString() << " " << path_ << " " << "HTTP/" << getVersion() << "\r\n";
    
    for (const auto& h : getHeaders()) {
        ss << h.first << ": " << h.second << "\r\n";
    }

    ss << "\r\n";

    ss << getBody() << "\r\n";

    return ss.str();
}

HttpMethod Request::getMethod() const {
    return method_;
}

const std::string& Request::getPath() const {
    return path_;
}

const std::string& Request::getRawRequest() const {
    return rawRequest_;
}

void Request::setMethod(const std::string& method) {
    try {
        method_ = HttpMethod(method);
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to set http method");
    }
}

void Request::setMethod(const METHOD method) {
    method_ = std::move(HttpMethod(method));
}

void Request::setPath(std::string path) {
    if (path.empty() || path.at(0) != '/') {
        throw std::runtime_error("Path is invalid");
    }

    path_ = std::move(path);
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

static constexpr std::string_view headerEndDelim    = "\r\n\r\n";
static const std::string_view clString              = "Content-Length:";

uint16_t Request::headersReceived(std::string_view data, std::size_t& headerPos) {
    headerPos = data.find(headerEndDelim);
    if (headerPos == std::string::npos) return 1;

    return 0;
}

uint16_t Request::bodyReceived(std::string_view data, std::size_t& headerPos) {
    auto clPos = data.find(clString);
    if (clPos != std::string::npos && clPos < headerPos) {
        auto valStart = clPos + clString.size();
        auto valEnd = data.find("\r\n", valStart);
        size_t contentLength = std::stoi(std::string(data.substr(valStart, valEnd - valStart)));
        
        // Check if body is complete
        if (data.size() < headerPos + headerEndDelim.size() + contentLength) return 1; // not complete;
        if (data.size() > headerPos + headerEndDelim.size() + contentLength) {
            return 413; // message bigger than assigned in contentlength
        }
    } else if (clPos != std::string::npos) {
        return 406; // content length assigned outside of headers
    }

    return 0;
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

/**
 * ------------------------------------------------------------------------
 * ------------------------ RESPONSE --------------------------------------
 * ------------------------------------------------------------------------
 */
Response::Response() {
    baseConfiguration();
}

Response::Response(
    RESPONSE_CODE code,
    std::initializer_list<std::pair<std::string, std::string>> headers,
    std::string body
) {
    setResponseCode(code);
    addHeaders(headers);
    setBody(body);

    baseConfiguration();
}

Response::~Response() {}

std::string Response::buildResponse() {
    if (getBody().size() > 0) {
        if (getHeaders().find("Content-Type") == getHeaders().end()) {
            throw std::invalid_argument("Response has payload but no Content-Type");
        }
    }
    setHeader("Content-Length", std::to_string(getBody().size()));

    std::stringstream ss;

    ss << "HTTP/" << getVersion() << " " << getResponseCode().getCodeString() << " " << getResponseCode().getMessageString() << "\r\n";

    for (const auto& [k, v] : getHeaders()) {
        ss << k << ": " << v << "\r\n";
    }
    ss << "\r\n";

    ss << getBody();

    return ss.str();
}

void Response::setResponseCode(RESPONSE_CODE code) {
    responseCode_ = HttpResponseCode(code);
}

HttpResponseCode Response::getResponseCode() const {
    return responseCode_;
}

void Response::baseConfiguration() {
    setVersion("1.1");

    setHeader("Server", LTALE_SERVER_NAME);

    time_t timestamp = time(&timestamp);
    struct tm datetime = *localtime(&timestamp);

    std::string date = ctime(&timestamp);
    date.pop_back();  // remove the '\n' ctime appends
    setHeader("Date", std::move(date));
}

} // namespace end
