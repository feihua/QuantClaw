#include "quantclaw/cli/agent_commands.hpp"
#include "quantclaw/gateway/gateway_client.hpp"
#include "quantclaw/gateway/protocol.hpp"
#include <iostream>

namespace quantclaw::cli {

AgentCommands::AgentCommands(std::shared_ptr<spdlog::logger> logger)
    : logger_(logger) {
}

int AgentCommands::request_command(const std::vector<std::string>& args) {
    std::string message;
    std::string session_key = "agent:default:main";
    std::string model;
    bool json_output = false;

    for (size_t i = 0; i < args.size(); ++i) {
        if ((args[i] == "-m" || args[i] == "--message") && i + 1 < args.size()) {
            message = args[++i];
        } else if ((args[i] == "-s" || args[i] == "--session") && i + 1 < args.size()) {
            session_key = args[++i];
        } else if (args[i] == "--model" && i + 1 < args.size()) {
            model = args[++i];
        } else if (args[i] == "--json") {
            json_output = true;
        } else if (args[i][0] != '-' && message.empty()) {
            // Positional argument as message
            message = args[i];
        }
    }

    if (message.empty()) {
        std::cerr << "Error: message required. Use: quantclaw agent -m \"your message\"" << std::endl;
        return 1;
    }

    try {
        auto client = std::make_shared<gateway::GatewayClient>(gateway_url_, "", logger_);
        if (!client->connect()) {
            std::cerr << "Error: Cannot connect to gateway at " << gateway_url_ << std::endl;
            std::cerr << "Is the gateway running? Start it with: quantclaw gateway" << std::endl;
            return 1;
        }

        // Subscribe to streaming events
        client->subscribe("agent.text_delta", [](const std::string&, const nlohmann::json& payload) {
            if (payload.contains("text")) {
                std::cout << payload["text"].get<std::string>() << std::flush;
            }
        });

        client->subscribe("agent.message_end", [](const std::string&, const nlohmann::json&) {
            std::cout << std::endl;
        });

        // Make agent.request RPC call
        nlohmann::json params = {
            {"sessionKey", session_key},
            {"message", message}
        };
        if (!model.empty()) {
            params["model"] = model;
        }

        auto result = client->call("agent.request", params, 120000);

        if (json_output) {
            std::cout << result.dump(2) << std::endl;
        }

        client->disconnect();
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int AgentCommands::stop_command(const std::vector<std::string>& /*args*/) {
    try {
        auto client = std::make_shared<gateway::GatewayClient>(gateway_url_, "", logger_);
        if (!client->connect()) {
            std::cerr << "Error: Cannot connect to gateway" << std::endl;
            return 1;
        }

        auto result = client->call("agent.stop", {});
        std::cout << "Agent stopped" << std::endl;

        client->disconnect();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

} // namespace quantclaw::cli
