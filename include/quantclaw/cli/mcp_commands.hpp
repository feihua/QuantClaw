#pragma once

#include <string>
#include <vector>
#include "quantclaw/mcp/mcp_server.hpp"
#include "quantclaw/mcp/mcp_client.hpp"
#include <spdlog/spdlog.h>

namespace quantclaw::cli {

class MCPCommands {
private:
    std::shared_ptr<spdlog::logger> logger_;
    
public:
    explicit MCPCommands(std::shared_ptr<spdlog::logger> logger);
    
    // MCP Server commands
    int mcp_server_start_command(const std::vector<std::string>& args);
    int mcp_server_status_command(const std::vector<std::string>& args);
    
    // MCP Client commands  
    int mcp_client_list_tools_command(const std::vector<std::string>& args);
    int mcp_client_call_tool_command(const std::vector<std::string>& args);
    
private:
    void parse_server_args(const std::vector<std::string>& args, int& port, std::string& host);
    void parse_client_args(const std::vector<std::string>& args, std::string& server_url);
};

} // namespace quantclaw::cli