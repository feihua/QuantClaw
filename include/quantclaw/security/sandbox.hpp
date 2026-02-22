#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <regex>

namespace quantclaw {

class Sandbox {
private:
    std::filesystem::path workspace_path_;
    std::vector<std::string> allowed_paths_;
    std::vector<std::string> denied_paths_;
    std::vector<std::string> allowed_commands_;
    std::vector<std::string> denied_commands_;
    std::vector<std::regex> denied_cmd_patterns_;

public:
    Sandbox(const std::filesystem::path& workspace_path,
           const std::vector<std::string>& allowed_paths,
           const std::vector<std::string>& denied_paths,
           const std::vector<std::string>& allowed_commands,
           const std::vector<std::string>& denied_commands);

    bool is_path_allowed(const std::string& path) const;
    bool is_command_allowed(const std::string& command) const;
    std::string sanitize_path(const std::string& path) const;

    // Static convenience methods used by ToolRegistry and MCPServer
    static bool validate_file_path(const std::string& path, const std::string& workspace);
    static bool validate_shell_command(const std::string& command);
    static void apply_resource_limits();
};

using SecuritySandbox = Sandbox;

} // namespace quantclaw
