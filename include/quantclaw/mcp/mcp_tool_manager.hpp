#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "quantclaw/config.hpp"
#include "quantclaw/mcp/mcp_client.hpp"

namespace quantclaw {

class ToolRegistry;

namespace mcp {

class MCPToolManager {
public:
    explicit MCPToolManager(std::shared_ptr<spdlog::logger> logger);

    // Connect to all configured MCP servers and discover their tools
    void discover_tools(const MCPConfig& config);

    // Register discovered tools into a ToolRegistry
    void register_into(ToolRegistry& registry);

    // Execute an external tool by its qualified name
    std::string execute_tool(const std::string& qualified_name,
                             const nlohmann::json& arguments);

    // Name resolution helpers
    bool is_external_tool(const std::string& name) const;
    std::string get_server_name(const std::string& qualified_name) const;
    std::string get_original_tool_name(const std::string& qualified_name) const;

    // Get count of discovered tools
    size_t tool_count() const { return tool_to_server_.size(); }

    // Build qualified name: mcp__{server}__{tool}
    static std::string make_qualified_name(const std::string& server_name,
                                           const std::string& tool_name);

private:
    std::shared_ptr<spdlog::logger> logger_;

    // server_name -> MCPClient
    std::unordered_map<std::string, std::shared_ptr<MCPClient>> clients_;

    // qualified_name -> server_name
    std::unordered_map<std::string, std::string> tool_to_server_;

    // qualified_name -> original tool name on the MCP server
    std::unordered_map<std::string, std::string> tool_to_original_name_;

    // qualified_name -> tool metadata (description, parameters)
    struct ToolMeta {
        std::string description;
        nlohmann::json parameters;
    };
    std::unordered_map<std::string, ToolMeta> tool_meta_;
};

} // namespace mcp
} // namespace quantclaw
