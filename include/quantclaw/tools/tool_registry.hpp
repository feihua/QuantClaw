#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "quantclaw/config.hpp"

namespace quantclaw {

class ToolPermissionChecker;

namespace mcp {
    class MCPToolManager;
}

class ToolRegistry {
public:
    struct ToolSchema {
        std::string name;
        std::string description;
        nlohmann::json parameters;
    };

private:
    std::shared_ptr<spdlog::logger> logger_;
    std::unordered_map<std::string, std::function<std::string(const nlohmann::json&)>> tools_;
    std::vector<ToolSchema> tool_schemas_;
    std::shared_ptr<ToolPermissionChecker> permission_checker_;
    std::shared_ptr<mcp::MCPToolManager> mcp_tool_manager_;
    std::unordered_set<std::string> external_tools_;  // tracks MCP tool names

public:
    explicit ToolRegistry(std::shared_ptr<spdlog::logger> logger);

    // Register built-in tools (compatible with OpenClaw)
    void register_builtin_tools();

    // Register an external tool (from MCP server)
    void register_external_tool(const std::string& name,
                                const std::string& description,
                                const nlohmann::json& parameters,
                                std::function<std::string(const nlohmann::json&)> executor);

    // Register the chain meta-tool
    void register_chain_tool();

    // Set permission checker (filters get_tool_schemas and execute_tool)
    void set_permission_checker(std::shared_ptr<ToolPermissionChecker> checker);

    // Set MCP tool manager (for permission checks on external tools)
    void set_mcp_tool_manager(std::shared_ptr<mcp::MCPToolManager> manager);

    // Execute a tool by name (with permission check)
    std::string execute_tool(const std::string& tool_name,
                            const nlohmann::json& parameters);

    // Get tool schemas for LLM function calling (filtered by permissions)
    std::vector<ToolSchema> get_tool_schemas() const;

    // Check if tool is available
    bool has_tool(const std::string& tool_name) const;

private:
    // Permission check helper
    bool check_permission(const std::string& tool_name) const;

    // Built-in tool implementations
    std::string read_file_tool(const nlohmann::json& params);
    std::string write_file_tool(const nlohmann::json& params);
    std::string edit_file_tool(const nlohmann::json& params);
    std::string exec_tool(const nlohmann::json& params);
    std::string message_tool(const nlohmann::json& params);
};

} // namespace quantclaw
