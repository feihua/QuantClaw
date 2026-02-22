#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "quantclaw/config.hpp"

namespace quantclaw {

struct SkillMetadata {
    std::string name;
    std::string description;
    std::vector<std::string> required_bins;
    std::vector<std::string> required_envs;
    std::vector<std::string> any_bins;        // at least one must exist
    std::vector<std::string> config_files;    // required config files
    std::vector<std::string> os_restrict;     // e.g. ["linux", "darwin", "win32"]
    bool always = false;                      // skip all gating
    std::string primary_env;                  // primary environment variable
    std::string emoji;                        // display emoji
    std::string content;
};

class SkillLoader {
public:
    explicit SkillLoader(std::shared_ptr<spdlog::logger> logger);

    // Load skills from directory (compatible with OpenClaw SKILL.md format)
    std::vector<SkillMetadata> load_skills_from_directory(
        const std::filesystem::path& skills_dir
    );

    // Multi-directory loading with dedup and config filtering
    std::vector<SkillMetadata> load_skills(
        const SkillsConfig& skills_config,
        const std::filesystem::path& workspace_path);

    // Check if skill can be loaded based on environment (gating)
    bool check_skill_gating(const SkillMetadata& skill);

    // Get skill content for LLM context
    std::string get_skill_context(const std::vector<SkillMetadata>& skills) const;

private:
    // Parse SKILL.md file
    SkillMetadata parse_skill_file(const std::filesystem::path& skill_file) const;

    // Parse YAML frontmatter with indent-aware nesting support
    nlohmann::json parse_yaml_frontmatter(const std::string& yaml_str) const;

    // Check if binary exists in PATH
    bool is_binary_available(const std::string& binary_name) const;

    // Check if environment variable exists
    bool is_env_var_available(const std::string& env_var) const;

    // Check current OS against restriction list
    bool check_os_restriction(const std::vector<std::string>& os_list) const;

    // Get current OS identifier
    std::string get_current_os() const;

    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace quantclaw
