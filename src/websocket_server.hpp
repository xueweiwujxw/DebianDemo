/**
 * @file websocket_server.hpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief WebSocket服务器类的头文件
 * @date 2024-07-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <ixwebsocket/IXWebSocketServer.h>
#include <functional>
#include <string>
#include <nlohmann/json.hpp>
#include <map>
#include <mutex>
#include <chrono>
#include <future>

#include <log.h>

namespace websocketnp
{
    /**
     * @class WebsocketServer
     * @brief 用于处理WebSocket连接和消息的服务器类
     */
    class WebsocketServer
    {
    public:
        /// 定义消息回调类型
        using MessageCallback = std::function<nlohmann::json(const nlohmann::json &)>;

        /**
         * @brief 构造函数
         *
         * @param port 监听的端口号
         * @param host 监听的主机名
         */
        WebsocketServer(int port,
                         std::string host,
                         std::chrono::seconds timeout_duration = std::chrono::seconds(5)) : server(port, host), timeout_duration(timeout_duration), running(false) {}

        /**
         * @brief 析构函数
         *
         */
        ~WebsocketServer() {}

        /**
         * @brief 启动WebSocket服务器
         */
        void start() {
            this->server.setOnClientMessageCallback([this](std::shared_ptr<ix::ConnectionState> connection_state, ix::WebSocket &websocket, const ix::WebSocketMessagePtr &msg) {
                this->handle_message(connection_state, websocket, msg);
            });
            this->running = true;
            this->server.listen();
            this->server.start();

            if (this->timeout_duration.count() > 0)
                this->timeout_future = std::async(&WebsocketServer::check_timeouts, this);
        }

        /**
         * @brief 停止WebSocket服务器
         */
        void stop() {
            this->server.stop();
            this->running = false;
            if (this->timeout_future.valid())
                this->timeout_future.wait();
        }

        /**
         * @brief 广播消息给所有连接的客户端
         *
         * @param msg 要广播的JSON消息
         */
        void brodcast_message(nlohmann::json &&msg) {
            std::lock_guard<std::mutex> lock(this->websocket_mutex);
            std::for_each(this->websockets.begin(), this->websockets.end(), [msg](const std::pair<const std::string, std::pair<std::pair<ix::WebSocket *, std::string>, std::chrono::steady_clock::time_point>> &entry) {
                entry.second.first.first->send(msg.dump());
            });
        }

        /**
         * @brief 注册消息回调函数
         *
         * @param key 回调函数的键
         * @param callback 回调函数
         */
        void register_callbacks(std::string key, MessageCallback callback) {
            this->callbacks.insert(std::make_pair(key, callback));
        }

        /**
         * @brief 注销消息回调函数
         *
         * @param key 要注销的回调函数的键
         */
        void unregister_callbacks(std::string key) {
            this->callbacks.erase(key);
        }

        /**
         * @brief 输出记录的所有连接信息
         *
         * @return nlohmann::json
         */
        nlohmann::json show_all_connections() {
            nlohmann::json j;
            std::for_each(this->websockets.begin(), this->websockets.end(), [&j](const std::pair<const std::string, std::pair<std::pair<ix::WebSocket *, std::string>, std::chrono::steady_clock::time_point>> &entry) {
                j.push_back({{"id", entry.first}, {"url", entry.second.first.second}});
            });
            return j;
        }

        bool is_running() {
            return this->running.load();
        }

    private:
        ix::WebSocketServer server;                                                                                                  // WebSocket服务器实例
        std::map<std::string, MessageCallback> callbacks;                                                                           // 消息回调函数映射表
        std::map<std::string, std::pair<std::pair<ix::WebSocket *, std::string>, std::chrono::steady_clock::time_point>> websockets; // WebSocket连接映射表
        std::mutex websocket_mutex;                                                                                                  // 保护WebSocket连接映射表的互斥锁
        std::mutex callback_mutex;                                                                                                   // 回调函数互斥量
        std::chrono::seconds timeout_duration;                                                                                       // 超时时间
        std::future<void> timeout_future;                                                                                            // 超时检查的future
        std::atomic<bool> running;                                                                                                   // 用于控制超时检查的运行

        /**
         * @brief 处理接收消息
         *
         * @param connection_state
         * @param websocket
         * @param msg
         */
        void handle_message(std::shared_ptr<ix::ConnectionState> connection_state, ix::WebSocket &websocket, const ix::WebSocketMessagePtr &msg) {
            switch (msg->type) {
            case ix::WebSocketMessageType::Message: {
                try {
                    auto json_msg = nlohmann::json::parse(msg->str);
                    if (!json_msg.contains("value") || !json_msg.contains("type")) {
                        nlohmann::json ret = {{"error", "Wrong JSON format"}};
                        websocket.send(ret.dump());
                        this->update_last_active_time(connection_state->getId());
                        return;
                    }
                    std::string parse_type = json_msg.value("type", "");
                    if (parse_type == "ping") {
                        nlohmann::json ret = {{"type", "pong"}};
                        websocket.send(ret.dump());
                        this->update_last_active_time(connection_state->getId());
                        return;
                    }
                    if (this->callbacks.find(parse_type) != this->callbacks.end()) {
                        {
                            std::lock_guard<std::mutex> lock(this->callback_mutex);
                            auto ret = this->callbacks[parse_type](json_msg["value"]);
                            websocket.send(ret.dump());
                        }
                    } else {
                        logf_warn("%s.\n", parse_type.c_str());
                        nlohmann::json ret = {{"error", "Unknown type: " + parse_type}};
                        websocket.send(ret.dump());
                    }

                    this->update_last_active_time(connection_state->getId());
                } catch (const std::exception &e) {
                    nlohmann::json ret = {{"error", e.what()}};
                    websocket.send(ret.dump());
                    logf_warn("Invalid JSON message: %s\n", e.what());
                }
                break;
            }
            case ix::WebSocketMessageType::Open: {
                {
                    std::lock_guard<std::mutex> lock(this->websocket_mutex);
                    this->websockets[connection_state->getId()] = {{&websocket, connection_state->getRemoteIp() + ":" + std::to_string(connection_state->getRemotePort())}, std::chrono::steady_clock::now()};
                }
                logf_info("%s:%d %s connected.\n", connection_state->getRemoteIp().c_str(), connection_state->getRemotePort(), connection_state->getId().c_str());
                break;
            }
            case ix::WebSocketMessageType::Close: {
                {
                    std::lock_guard<std::mutex> lock(this->websocket_mutex);
                    this->websockets.erase(connection_state->getId());
                }
                logf_info("%s:%d %s disconnected.\n", connection_state->getRemoteIp().c_str(), connection_state->getRemotePort(), connection_state->getId().c_str());
                break;
            }
            default:
                break;
            }
        }

        /**
         * @brief 更新活跃时间
         *
         * @param id
         */
        void update_last_active_time(const std::string &id) {
            std::lock_guard<std::mutex> lock(this->websocket_mutex);
            if (this->websockets.find(id) != this->websockets.end()) {
                this->websockets[id].second = std::chrono::steady_clock::now();
            }
        }

        /**
         * @brief 超时检测
         *
         */
        void check_timeouts() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::seconds(2));

                auto now = std::chrono::steady_clock::now();
                std::lock_guard<std::mutex> lock(this->websocket_mutex);
                for (auto it = this->websockets.begin(); it != this->websockets.end();) {
                    if (now - it->second.second > timeout_duration) {
                        logf_info("Closing connection: %s %s due to timeout\n", it->second.first.second.c_str(), it->first.c_str());
                        it->second.first.first->close();
                        it = this->websockets.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
    };
} // namespace websocket
