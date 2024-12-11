/**
 * @file http_server.cpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief Implementation of the HttpServer class.
 *
 * This file contains the implementation of the HttpServer class, which provides a simple HTTP server implementation based on the httplib library.
 *
 * @date 2023-02-03
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <http_server.hpp>
#include <log.h>

using namespace httpservernp;

HttpServer::HttpServer(const int &port, const std::string &host) : webpath(WEB_HOME),
                                                                   port(port),
                                                                   host(host) {}

HttpServer::~HttpServer() {}

bool HttpServer::register_handler(std::string path, HttpMethods method, http_handler handler) {
    switch (method) {
    case HttpMethods::GET:
        this->srv.Get(path, handler);
        return true;
    case HttpMethods::POST:
        this->srv.Post(path, handler);
        return true;
    default:
        int method_size = sizeof(http_methods) / sizeof(http_methods[0]);
        logf_warn("not supported method: %s\n", (int(method) < method_size && int(method) >= 0) ? http_methods[int(method)].c_str() : "Unknown");
        return false;
    }
}

bool HttpServer::register_handler(std::string path, HttpMethods method, http_handler_with_content handler) {
    switch (method) {
    case HttpMethods::POST:
        this->srv.Post(path, handler);
        return true;
    default:
        return false;
    }
}

void HttpServer::set_error_handler(http_handler handler) {
    this->srv.set_error_handler(handler);
}

void HttpServer::set_exception_handler(httplib::Server::ExceptionHandler handler) {
    this->srv.set_exception_handler(handler);
}

void HttpServer::set_root_path(const std::string &webpath) {
    this->webpath = webpath;
}

void HttpServer::start() {
    this->run_future = std::async(&HttpServer::run, this);
}

void HttpServer::enable_cross_domain() {
    this->srv.set_default_headers({{"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
                                   {"Access-Control-Allow-Headers", "*"},
                                   {"Access-Control-Allow-Origin", "*"}});

    this->srv.Options(R"(.*)", [](const httplib::Request &req, httplib::Response &res) {
        res.status = 200;
    });
}

void HttpServer::custom_options(const std::string &pattern, httplib::Server::Handler handler) {
    this->srv.Options(pattern, handler);
}

void HttpServer::set_keep_alive_max_count(size_t count) {
    this->srv.set_keep_alive_max_count(count);
}

void HttpServer::set_keep_alive_timeout(time_t sec) {
    this->srv.set_keep_alive_timeout(sec);
}

void HttpServer::stop() {
    this->srv.stop();
    this->run_future.wait();
    logf_info("stop http server\n");
}

bool HttpServer::run() {
    bool ret = this->srv.set_mount_point("/", this->webpath);
    if (!ret) {
        logf_err("set mount point '/' to %s failed.\n", this->webpath.c_str());
        return ret;
    }
    logf_info("start http server\n");
    logf_info("started to listen http://%s:%d\n", host.c_str(), port);
    ret = this->srv.listen(host, port);
    return ret;
}
