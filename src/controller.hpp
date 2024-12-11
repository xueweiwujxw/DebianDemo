/**
 * @file controller.hpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @date 2024-10-23
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <atomic>
#include <future>

#include <websocket_server.hpp>
#include <http_server.hpp>

namespace demonp
{
    class Controller
    {
    private:
        websocketnp::WebsocketServer ws; // websocket server
        httpservernp::HttpServer hs;     // http server

        std::atomic<bool> is_running;      // 运行标志
        std::atomic<bool> working;         // 设备工作状态
        std::future<void> stat_rpt_future; // 状态上报线程

        /**
         * @brief 初始化软硬件
         *
         * @return true
         * @return false
         */
        bool init();

        /**
         * @brief 回收软硬件初始化资源
         *
         */
        void deinit();

        /*-- websocket 接口 --*/
        /**
         * @brief 处理开始工作请求
         *
         * @param cmd
         * @return nlohmann::json
         */
        nlohmann::json handle_start_work(const nlohmann::json &cmd);

        /**
         * @brief 处理停止工作请求
         *
         * @param cmd
         * @return nlohmann::json
         */
        nlohmann::json handle_stop_work(const nlohmann::json &cmd);

        /**
         * @brief 获取工作状态
         *
         * @param cmd
         * @return nlohmann::json
         */
        nlohmann::json handle_get_working(const nlohmann::json &cmd);

        /**
         * @brief 状态周期性上报
         *
         */
        void status_report_looper();

    public:
        /**
         * @brief controller构造函数
         *
         * @param wsport websocket服务端口号
         * @param hsport http服务端口号
         * @param host 服务主机地址
         */
        Controller(int wsport, int hsport, std::string host);
        ~Controller();

        /**
         * @brief 启动服务
         *
         * @param slot_alias 插槽别名
         * @param axi_byte_width bus字节位宽
         * @return true
         * @return false
         */
        bool start();

        /**
         * @brief 停止服务
         *
         */
        void stop();
    };

} // namespace demonp
