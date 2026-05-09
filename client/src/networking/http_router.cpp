#include "../../include/networking/http_router.hpp"
#include "../../include/networking/networking_config.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace networking::http {

    static inline const std::unordered_map<std::string, std::string> contentTypes = {
        {"unkown",  "text/plain"},
        {"js",      "text/javascript"},
        {"css",     "text/css"},
        {"html",    "text/html"},
        {"json",    "application/json"},
        {"jpeg",    "image/jpeg"},
        {"png",     "image/png"},
        {"ico",     "image/png"}
    };

    HttpRouter::HttpRouter() {
        scanResources(LTALE_HTTP_SERVER_RESOURCE_PATH);
    }

    void HttpRouter::scanResources(const std::string& directory) {
        if (!fs::is_directory(directory)) {
            throw std::runtime_error("Unable to locate resource path");
        }

        for (const auto& f : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(f)) {
                if (f.path().filename().generic_string().find('.') == std::string::npos) return;
                availableResources.emplace(
                    f.path().generic_string(),
                    fs::path(f).generic_string()
                );
            } else if (fs::is_directory(f)) {
                scanResources(f.path().generic_string());
            }
        }

        return;
    }

    Response HttpRouter::handleRequest(const Request& request) {
        if (request.getMethod().getMethodEnum() == METHOD::GET) {
            return routeGetRequest(request);
        }

        return Response(RESPONSE_CODE::NOT_FOUND, {}, {"There is no file with that name"});
    }

    Response HttpRouter::routeGetRequest(const Request& request) {
        try {
            std::string path = request.getPath();
            if (auto q = path.find('?'); q != std::string::npos)
                path.erase(q);
            if (path == "/") path = "/index.html";

            Response response;
            std::string body;
            
            if (fetchContent(path, body) != 0) {
                std::string notFoundBody;
                fetchContent("/404.html", notFoundBody);
                response.setBody(notFoundBody);
                response.setHeader("Content-Type", "text/html");
                response.setResponseCode(RESPONSE_CODE::NOT_FOUND);
                return response;
            }
            response.setBody(body);
            response.setHeader("Content-Type", parseContentType(path));
            response.setResponseCode(RESPONSE_CODE::OK);

            return response;
        } catch (const std::exception& e) {
            return Response(RESPONSE_CODE::INTERNAL_SERVER_ERROR, {}, {});
        }
    }

    uint8_t HttpRouter::fetchContent(const std::string& path, std::string& contentOut) {
        std::string resourcePath = std::string(LTALE_HTTP_SERVER_RESOURCE_PATH) + path;
        std::ifstream file(resourcePath, std::ios::binary);

        if (!file.is_open()) {
            return 1;
        }
        
        contentOut.append(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        );

        file.close();
        return 0;
    }

    std::string HttpRouter::parseContentType(const std::string& fName) {
        std::string contentType;
        std::stringstream ss(fName);

        while (std::getline(ss, contentType, '.'));
        while (auto pos = contentType.find('.') != std::string::npos) {
            contentType.erase(contentType.begin() + pos);
        }

        return contentTypes.at(contentType);
    }
}
