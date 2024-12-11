/**
 * @file websocket_server_test.cc
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @date 2024-07-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <websocket_server.hpp>
#include <iostream>
#include <atomic>
#include <future>
#include <string>

using namespace websocketnp;

nlohmann::json geta() {
    return {{"value", "fake_val"}};
}

int main(int argc, char const *argv[]) {
    srand(time(nullptr));
    WebsocketServer server(9000, "0.0.0.0", std::chrono::seconds(2));
    std::atomic<bool> running;
    running = true;
    std::future<void> fu = std::async([&running, &server]() {
        while (running) {
            server.brodcast_message(geta());
            sleep(1);
        }
    });

    server.register_callbacks("a", [](const nlohmann::json &msg) {
        logf_info("%s\n", msg.dump().c_str());
        return nlohmann::json();
    });

    server.start();

    logf_info("webserver started.\n");
    printf("press q with enter to quit.\n");
    printf("press s with enter to show connections.\n");
    printf("press t with enter to send test message.\n");
    char quit;
    while (true) {
        quit = getchar();
        if (quit == 's')
            logf_info("\n%s\n", server.show_all_connections().dump(4).c_str());
        else if (quit == 'q')
            break;
        else if (quit == 't')
            server.brodcast_message({{"type", "CONSOLE"}, {"value", {{"message", "SNR = " + std::to_string(random() % 80 - 40) + " dB"}, {"level", "INFO"}}}});
    }

    running = false;
    fu.wait();

    server.stop();
    logf_info("webserver stopped.\n");
    return 0;
}
