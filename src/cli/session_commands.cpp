#include "quantclaw/cli/session_commands.hpp"
#include "quantclaw/gateway/gateway_client.hpp"
#include <iostream>
#include <iomanip>

namespace quantclaw::cli {

SessionCommands::SessionCommands(std::shared_ptr<spdlog::logger> logger)
    : logger_(logger) {
}

int SessionCommands::list_command(const std::vector<std::string>& args) {
    bool json_output = false;
    int limit = 20;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--json") {
            json_output = true;
        } else if (args[i] == "--limit" && i + 1 < args.size()) {
            limit = std::stoi(args[++i]);
        }
    }

    try {
        auto client = std::make_shared<gateway::GatewayClient>(gateway_url_, "", logger_);
        if (!client->connect()) {
            std::cerr << "Error: Cannot connect to gateway" << std::endl;
            return 1;
        }

        auto result = client->call("sessions.list", {{"limit", limit}});

        if (json_output) {
            std::cout << result.dump(2) << std::endl;
        } else {
            if (result.is_array() && result.empty()) {
                std::cout << "No sessions found" << std::endl;
            } else if (result.is_array()) {
                std::cout << std::left
                          << std::setw(35) << "KEY"
                          << std::setw(15) << "ID"
                          << std::setw(25) << "UPDATED"
                          << std::setw(20) << "NAME"
                          << std::endl;
                std::cout << std::string(95, '-') << std::endl;

                for (const auto& session : result) {
                    std::cout << std::left
                              << std::setw(35) << session.value("key", "")
                              << std::setw(15) << session.value("id", "")
                              << std::setw(25) << session.value("updatedAt", "")
                              << std::setw(20) << session.value("displayName", "")
                              << std::endl;
                }
            }
        }

        client->disconnect();
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int SessionCommands::history_command(const std::vector<std::string>& args) {
    std::string session_key;
    bool json_output = false;
    int limit = -1;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--json") {
            json_output = true;
        } else if (args[i] == "--limit" && i + 1 < args.size()) {
            limit = std::stoi(args[++i]);
        } else if (args[i][0] != '-' && session_key.empty()) {
            session_key = args[i];
        }
    }

    if (session_key.empty()) {
        std::cerr << "Error: session key required" << std::endl;
        std::cerr << "Usage: quantclaw sessions history <session-key>" << std::endl;
        return 1;
    }

    try {
        auto client = std::make_shared<gateway::GatewayClient>(gateway_url_, "", logger_);
        if (!client->connect()) {
            std::cerr << "Error: Cannot connect to gateway" << std::endl;
            return 1;
        }

        nlohmann::json params = {{"sessionKey", session_key}};
        if (limit > 0) params["limit"] = limit;

        auto result = client->call("sessions.history", params);

        if (json_output) {
            std::cout << result.dump(2) << std::endl;
        } else if (result.is_array()) {
            for (const auto& msg : result) {
                std::string role = msg.value("role", "unknown");
                std::string content = msg.value("content", "");
                std::string ts = msg.value("timestamp", "");

                if (role == "user") {
                    std::cout << "\033[36m[" << ts << "] User:\033[0m " << content << std::endl;
                } else if (role == "assistant") {
                    std::cout << "\033[32m[" << ts << "] Assistant:\033[0m " << content << std::endl;
                } else {
                    std::cout << "[" << ts << "] " << role << ": " << content << std::endl;
                }
                std::cout << std::endl;
            }
        }

        client->disconnect();
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int SessionCommands::delete_command(const std::vector<std::string>& args) {
    std::string session_key;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i][0] != '-' && session_key.empty()) {
            session_key = args[i];
        }
    }

    if (session_key.empty()) {
        std::cerr << "Error: session key required" << std::endl;
        std::cerr << "Usage: quantclaw sessions delete <session-key>" << std::endl;
        return 1;
    }

    try {
        auto client = std::make_shared<gateway::GatewayClient>(gateway_url_, "", logger_);
        if (!client->connect()) {
            std::cerr << "Error: Cannot connect to gateway" << std::endl;
            return 1;
        }

        auto result = client->call("sessions.delete", {{"sessionKey", session_key}});
        client->disconnect();

        std::cout << "Session deleted: " << session_key << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int SessionCommands::reset_command(const std::vector<std::string>& args) {
    std::string session_key;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i][0] != '-' && session_key.empty()) {
            session_key = args[i];
        }
    }

    if (session_key.empty()) {
        std::cerr << "Error: session key required" << std::endl;
        std::cerr << "Usage: quantclaw sessions reset <session-key>" << std::endl;
        return 1;
    }

    try {
        auto client = std::make_shared<gateway::GatewayClient>(gateway_url_, "", logger_);
        if (!client->connect()) {
            std::cerr << "Error: Cannot connect to gateway" << std::endl;
            return 1;
        }

        auto result = client->call("sessions.reset", {{"sessionKey", session_key}});
        client->disconnect();

        std::cout << "Session reset: " << session_key << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

} // namespace quantclaw::cli
