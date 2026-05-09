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
    UNSET,
    GET,
    POST,
    PUT,
    DELETE
};

class HttpMethod {
public:
    HttpMethod() = default;
    HttpMethod(METHOD m);
    HttpMethod(std::string s);

    METHOD getMethodEnum();
    std::string getMethodString();

private:
    METHOD          eMethod_;
    std::string     sMethod_;
};

enum class RESPONSE_CODE : uint16_t {
    UNSET               = 0,
    OK                  = 200,
    CREATED             = 201,
    BAD_REQUEST         = 400,
    UNAUTHORIZED        = 401,
    NOT_FOUND           = 404,
    CONTENT_TOO_LARGE   = 413,
    NOT_ACCEPTABLE      = 406
};

class HttpResponseCode {
public:
    HttpResponseCode() = default;
    HttpResponseCode(RESPONSE_CODE r);
    HttpResponseCode(std::string r);

    RESPONSE_CODE getCodeEnum();
    std::string getCodeString();
    std::string getMessageString();

private:
    RESPONSE_CODE eCode;
    std::string sCode;
};

class HttpMessageBase {
public:
    HttpMessageBase() = default;
    virtual ~HttpMessageBase() = 0;

    const std::string& getVersion() const;
    const http_headers& getHeaders() const;
    const std::string& getBody() const;

    void setHeader(std::string key, std::string value);
    void addHeaders(std::initializer_list<std::pair<std::string, std::string>> headers);
    void setBody(const std::string& body);
    void setVersion(std::string version);

private:
    std::string         version_;
    http_headers        headers_;
    std::string         body_;
}; 

class Request : public HttpMessageBase {
public:
    Request() = default;
    Request(std::string rawRequest);
    Request(
        METHOD method, std::string path, std::string host,
        std::initializer_list<std::pair<std::string, std::string>> headers,
        std::string body
    );

    virtual ~Request() override;

    std::string getRequest();

    HttpMethod getMethod() const;
    const std::string& getPath() const;
    const std::string& getRawRequest() const;

    void setMethod(const std::string& method);
    void setMethod(const METHOD method);
    void setPath(std::string path);

    static uint16_t headersReceived(std::string_view data, std::size_t& pos);
    static uint16_t bodyReceived(std::string_view data, std::size_t& pos);

private:
    HttpMethod          method_;
    std::string         path_;
    std::string         rawRequest_;
    
    void parseRequest();
    void parseStartLine(std::string& line);
    void parseHeader(std::string& header);
};

class Response : public HttpMessageBase {
public:
    Response();
    Response(
        RESPONSE_CODE code,
        std::initializer_list<std::pair<std::string, std::string>> headers,
        std::string body
    );

    virtual ~Response() override;

    std::string buildResponse();

    void setResponseCode(RESPONSE_CODE code);
    HttpResponseCode getResponseCode() const;

private:
    HttpResponseCode    responseCode_;

    void baseConfiguration();
};
}

#endif