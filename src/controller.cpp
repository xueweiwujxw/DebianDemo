/**
 * @file controller.cpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @date 2024-10-23
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <fstream>

#include <controller.hpp>
#include <log.h>
#include <nlohmann/json.hpp>
#include <utils.hpp>

using namespace demonp;
using namespace std;
using namespace nlohmann;
using namespace utilsnp;

Controller::Controller(int wsport, int hsport, std::string host) : ws(wsport, host),
                                                                   hs(hsport, host),
                                                                   is_running(false),
                                                                   working(false) {}

Controller::~Controller() {
    this->deinit();
}

bool Controller::init() {
    try {
        // 初始化软硬件配置
        this->ws.register_callbacks("StartWork", bind(&Controller::handle_start_work, this, placeholders::_1));
        this->ws.register_callbacks("StopWork", bind(&Controller::handle_stop_work, this, placeholders::_1));
        this->ws.register_callbacks("Working", bind(&Controller::handle_get_working, this, placeholders::_1));
        this->ws.register_callbacks("VersionReq", [this](const nlohmann::json &msg) {
            json verinfo;
            verinfo["type"] = "OnVerInfo";
            json ctrlinfo;
#ifdef VERSION
            ctrlinfo["ver"] = VERSION;
#endif
#ifdef STAMP
            ctrlinfo["stamp"] = STAMP;
#endif
#ifdef HASH
            ctrlinfo["hash"] = HASH;
#endif
            if (ctrlinfo.size() == 3) {
                ctrlinfo["name"] = "服务";
                verinfo["value"].push_back(ctrlinfo);
            }

            return verinfo;
        });

        return true;
    } catch (const exception &e) {
        logf_err("%s\n", e.what());
        return false;
    }
}

void Controller::deinit() {}

json Controller::handle_start_work(const json &cmd) {
    this->working = true;
    return {
        {"type", "StartWorkRet"},
        {"value", {
                      {"success", true},
                      {"msg", ""},
                  }},
    };
}

json Controller::handle_stop_work(const json &cmd) {
    this->working = false;
    return {
        {"type", "StopWorkRet"},
        {"value", {
                      {"success", true},
                      {"msg", ""},
                  }},
    };
}

json Controller::handle_get_working(const nlohmann::json &cmd) {
    return {{"type", "WorkingRet"}, {"value", this->working.load()}};
}

void Controller::status_report_looper() {
    while (this->ws.is_running()) {
        json dev_stat = {};
        this->ws.brodcast_message({
            {"type", "DevStatRpt"},
            {"value", dev_stat},
        });

        sleep(1);
    }
}

bool Controller::start() {
    if (this->is_running) {
        logf_err("already running\n");
        return false;
    }

    if (!this->init()) {
        logf_err("initialize error.\n");
        return false;
    }
    this->ws.start();
    this->hs.start();

    this->stat_rpt_future = async(launch::async, &Controller::status_report_looper, this);

    this->is_running = true;

    return true;
}

void Controller::stop() {
    if (!this->is_running) {
        logf_warn("not runnning.\n");
        return;
    }

    this->ws.stop();
    this->hs.stop();
    this->stat_rpt_future.wait();
    this->deinit();

    this->is_running = false;
}