/**
 * @file http_server.hpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief Declaration of the HttpServer class.
 *
 * This file contains the declaration of the HttpServer class, which provides a simple HTTP server implementation based on the httplib library.
 *
 * @date 2023-02-03
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <httplib.h>
#include <future>

#ifndef WEB_HOME
#define WEB_HOME "/var/www/html"
#endif

using namespace httplib;

namespace httpservernp
{
    typedef httplib::Server::Handler http_handler;
    typedef httplib::Server::HandlerWithContentReader http_handler_with_content;

    /**
     * @brief Enum class representing HTTP methods.
     */
    enum class HttpMethods
    {
        GET, /** HTTP GET method */
        POST /** HTTP POST method */
    };

    inline std::string http_methods[] = {"GET", "POST"}; /** String representations of the HTTP methods. */

    /**
     * @brief HTTP server class.
     *
     * The HttpServer class provides a simple HTTP server implementation based on the httplib library.
     */
    class HttpServer
    {
    public:
        /**
         * @brief Register a handler for a specific path and HTTP method.
         *
         * @param path The URL path to register the handler for.
         * @param method The HTTP method to register the handler for.
         * @param handler The handler function to be called for the specified path and method.
         * @return True if the registration is successful, false otherwise.
         */
        bool register_handler(std::string path, HttpMethods method, http_handler handler);

        /**
         * @brief Register a handler with content reader for a specific path and HTTP method.
         *
         * @param path The URL path to register the handler for.
         * @param method The HTTP method to register the handler for.
         * @param handler The handler function with content reader to be called for the specified path and method.
         * @return True if the registration is successful, false otherwise.
         */
        bool register_handler(std::string path, HttpMethods method, http_handler_with_content handler);

        /**
         * @brief Start the HTTP server.
         *
         */
        void start();

        /**
         * @brief Stop the HTTP server.
         *
         */
        void stop();

        /**
         * @brief Set the error handler for the HTTP server.
         *
         * @param handler The error handler function to be called when an error occurs.
         */
        void set_error_handler(http_handler handler);

        /**
         * @brief Set the exception handler for the HTTP server.
         *
         * @param handler The exception handler function to be called when an exception occurs.
         */
        void set_exception_handler(httplib::Server::ExceptionHandler handler);

        /**
         * @brief set a web root path.
         *
         * @param webpath The path to the web root directory.
         */
        void set_root_path(const std::string &webpath);

        /**
         * @brief Constructor for the HttpServer class.
         *
         * @param port The port number to listen on.
         * @param host The host IP address or name to bind the server to.
         */
        HttpServer(const int &port, const std::string &host);

        /**
         * @brief Destructor for the HttpServer class.
         */
        ~HttpServer();

        /**
         * @brief Enable all cross domain
         */
        void enable_cross_domain();

        /**
         * @brief Customize server options for a specific pattern.
         *
         * @param pattern The URL pattern to match.
         * @param handler The handler function to call when a request matches the pattern.
         */
        void custom_options(const std::string &pattern, httplib::Server::Handler handler);

        /**
         * @brief Set the maximum number of keep-alive requests.
         *
         * @param count The maximum number of keep-alive requests. Must be bigger than 0
         */
        void set_keep_alive_max_count(size_t count);

        /**
         * @brief Set the maximum keep-alive timeout.
         *
         * @param sec The maximum keep-alive timeout in seconds.
         *            If set to 0, keep-alive is disabled.
         *            Default is 5 seconds.
         */
        void set_keep_alive_timeout(time_t sec);

    private:
        /**
         * @brief Run the HTTP server.
         *
         * @return True if the server is running successfully, false otherwise.
         */
        bool run();

        std::future<bool> run_future; /** The future object for the running server. */
        std::string webpath;          /** The path to the web root directory. */
        Server srv;                   /** The underlying HTTP server instance. */
        int port;                     /** The port number to listen on. */
        std::string host;             /** The host IP address or name to bind the server to. */
    };

} // namespace httpservernp
