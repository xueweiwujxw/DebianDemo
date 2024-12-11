/**
 * @file debian_demo.cc
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @date 2024-12-11
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <condition_variable>
#include <mutex>
#include <signal.h>

#include <cxxopts.hpp>
#include <log.h>

#include <controller.hpp>

using namespace std;
using namespace demonp;

mutex sig_mutex;
condition_variable sig_cv;

void sigint_cb_handler(int signum) {
    sig_cv.notify_all();
}

int main(int argc, char const *argv[]) {
    std::string host = "127.0.0.1";
    int ws_port = 8080;
    int hs_port = 80;
    cxxopts::Options options("debian-demo", "Debian Demo app usage: ");

    try {
        options.add_options()(
            "help,h", "show help information")(
            "version,v", "show version info")(
            "host", "server run host - default 127.0.0.1", cxxopts::value<std::string>())(
            "wsport", "websocket server run port - default 8080", cxxopts::value<int>())(
            "hsport", "http server run port - default 80", cxxopts::value<int>());

        options.show_positional_help();

        auto parsers = options.parse(argc, argv);

        if (parsers.count("help")) {
            printf("%s\n", options.help().c_str());
            return 0;
        }

        if (parsers.count("version")) {
#ifdef VERSION
            printf("version: %s\n", VERSION);
#endif
#ifdef STAMP
            char stamp[20], format[] = "%Y-%m-%d %H:%M:%S";
            double timeval;
            sscanf(STAMP, "%lf", &timeval);
            time_t tt = static_cast<time_t>(static_cast<unsigned int>(timeval));
            strftime(stamp, sizeof(stamp), format, localtime(&tt));
            printf("stamp: %s\n", stamp);
#endif
#ifdef HASH
            printf("hash: %s\n", HASH);
#endif
            return 0;
        }

        if (parsers.count("host"))
            host = parsers["host"].as<std::string>();

        if (parsers.count("wsport"))
            ws_port = parsers["wsport"].as<int>();

        if (parsers.count("hsport"))
            hs_port = parsers["hsport"].as<int>();

    } catch (const std::exception &e) {
        printf("%s\n", e.what());
        printf("%s\n", options.help().c_str());
        return 1;
    }

    Controller ct(ws_port, hs_port, host);

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sigint_cb_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    logf_info("service start.\n");

    if (!ct.start()) {
        logf_err("start controller failed.\n");
        return 1;
    }

    unique_lock<mutex> lock(sig_mutex);
    sig_cv.wait(lock);

    ct.stop();

    lock.unlock();
    logf_info("service exit.\n");

    return 0;
}
