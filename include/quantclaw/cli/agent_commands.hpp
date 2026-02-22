#pragma once

#include <string>
#include <vector>
#include <memory>
#include <spdlog/spdlog.h>

namespace quantclaw::cli {

class AgentCommands {
public:
    explicit AgentCommands(std::shared_ptr<spdlog::logger> logger);

    int request_command(const std::vector<std::string>& args);
    int stop_command(const std::vector<std::string>& args);

private:
    std::shared_ptr<spdlog::logger> logger_;
    std::string gateway_url_ = "ws://127.0.0.1:18789";
};

} // namespace quantclaw::cli
