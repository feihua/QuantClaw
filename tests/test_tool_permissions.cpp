#include <gtest/gtest.h>
#include "quantclaw/security/tool_permissions.hpp"

class ToolPermissionsTest : public ::testing::Test {
protected:
    quantclaw::ToolPermissionConfig make_config(
        const std::vector<std::string>& allow,
        const std::vector<std::string>& deny) {
        quantclaw::ToolPermissionConfig cfg;
        cfg.allow = allow;
        cfg.deny = deny;
        return cfg;
    }
};

// --- Basic allow/deny ---

TEST_F(ToolPermissionsTest, EmptyConfigAllowsAll) {
    auto checker = quantclaw::ToolPermissionChecker(make_config({}, {}));
    EXPECT_TRUE(checker.is_allowed("read"));
    EXPECT_TRUE(checker.is_allowed("write"));
    EXPECT_TRUE(checker.is_allowed("exec"));
    EXPECT_TRUE(checker.is_allowed("anything"));
}

TEST_F(ToolPermissionsTest, GroupFsAllowsFileTools) {
    auto checker = quantclaw::ToolPermissionChecker(make_config({"group:fs"}, {}));
    EXPECT_TRUE(checker.is_allowed("read"));
    EXPECT_TRUE(checker.is_allowed("write"));
    EXPECT_TRUE(checker.is_allowed("edit"));
    EXPECT_FALSE(checker.is_allowed("exec"));
    EXPECT_FALSE(checker.is_allowed("message"));
}

TEST_F(ToolPermissionsTest, GroupRuntimeAllowsExec) {
    auto checker = quantclaw::ToolPermissionChecker(make_config({"group:runtime"}, {}));
    EXPECT_TRUE(checker.is_allowed("exec"));
    EXPECT_FALSE(checker.is_allowed("read"));
    EXPECT_FALSE(checker.is_allowed("write"));
}

TEST_F(ToolPermissionsTest, GroupAllAllowsEverything) {
    auto checker = quantclaw::ToolPermissionChecker(make_config({"group:all"}, {}));
    EXPECT_TRUE(checker.is_allowed("read"));
    EXPECT_TRUE(checker.is_allowed("write"));
    EXPECT_TRUE(checker.is_allowed("edit"));
    EXPECT_TRUE(checker.is_allowed("exec"));
    EXPECT_TRUE(checker.is_allowed("message"));
}

TEST_F(ToolPermissionsTest, SingleToolAllow) {
    auto checker = quantclaw::ToolPermissionChecker(make_config({"tool:read"}, {}));
    EXPECT_TRUE(checker.is_allowed("read"));
    EXPECT_FALSE(checker.is_allowed("write"));
    EXPECT_FALSE(checker.is_allowed("exec"));
}

TEST_F(ToolPermissionsTest, DenyOverridesAllow) {
    auto checker = quantclaw::ToolPermissionChecker(make_config({"group:all"}, {"tool:exec"}));
    EXPECT_TRUE(checker.is_allowed("read"));
    EXPECT_TRUE(checker.is_allowed("write"));
    EXPECT_FALSE(checker.is_allowed("exec"));
}

TEST_F(ToolPermissionsTest, DenyGroupOverridesAllowGroup) {
    auto checker = quantclaw::ToolPermissionChecker(
        make_config({"group:all"}, {"group:runtime"}));
    EXPECT_TRUE(checker.is_allowed("read"));
    EXPECT_FALSE(checker.is_allowed("exec"));
}

TEST_F(ToolPermissionsTest, MultipleAllowGroups) {
    auto checker = quantclaw::ToolPermissionChecker(
        make_config({"group:fs", "group:runtime"}, {}));
    EXPECT_TRUE(checker.is_allowed("read"));
    EXPECT_TRUE(checker.is_allowed("write"));
    EXPECT_TRUE(checker.is_allowed("edit"));
    EXPECT_TRUE(checker.is_allowed("exec"));
    EXPECT_FALSE(checker.is_allowed("message"));
}

// --- MCP tool permissions ---

TEST_F(ToolPermissionsTest, McpAllowAllWhenConfigEmpty) {
    auto checker = quantclaw::ToolPermissionChecker(make_config({}, {}));
    EXPECT_TRUE(checker.is_mcp_tool_allowed("code-tools", "lint"));
    EXPECT_TRUE(checker.is_mcp_tool_allowed("data", "query"));
}

TEST_F(ToolPermissionsTest, McpAllowSpecificServer) {
    auto checker = quantclaw::ToolPermissionChecker(
        make_config({"group:all", "mcp:code-tools:*"}, {}));
    EXPECT_TRUE(checker.is_mcp_tool_allowed("code-tools", "lint"));
    EXPECT_TRUE(checker.is_mcp_tool_allowed("code-tools", "format"));
    EXPECT_FALSE(checker.is_mcp_tool_allowed("data", "query"));
}

TEST_F(ToolPermissionsTest, McpAllowSpecificTool) {
    auto checker = quantclaw::ToolPermissionChecker(
        make_config({"group:all", "mcp:code-tools:lint"}, {}));
    EXPECT_TRUE(checker.is_mcp_tool_allowed("code-tools", "lint"));
    EXPECT_FALSE(checker.is_mcp_tool_allowed("code-tools", "format"));
}

TEST_F(ToolPermissionsTest, McpDenyOverridesAllow) {
    auto checker = quantclaw::ToolPermissionChecker(
        make_config({"group:all", "mcp:code-tools:*"}, {"mcp:code-tools:dangerous"}));
    EXPECT_TRUE(checker.is_mcp_tool_allowed("code-tools", "lint"));
    EXPECT_FALSE(checker.is_mcp_tool_allowed("code-tools", "dangerous"));
}

TEST_F(ToolPermissionsTest, McpDenyEntireServer) {
    auto checker = quantclaw::ToolPermissionChecker(
        make_config({"mcp:*"}, {"mcp:untrusted:*"}));
    EXPECT_TRUE(checker.is_mcp_tool_allowed("trusted", "anything"));
    EXPECT_FALSE(checker.is_mcp_tool_allowed("untrusted", "anything"));
}

// --- Default config (from_json defaults) ---

TEST_F(ToolPermissionsTest, DefaultPermissionConfigFromJson) {
    nlohmann::json empty_json = nlohmann::json::object();
    auto cfg = quantclaw::ToolPermissionConfig::from_json(empty_json);
    // Default: allow group:fs and group:runtime
    auto checker = quantclaw::ToolPermissionChecker(cfg);
    EXPECT_TRUE(checker.is_allowed("read"));
    EXPECT_TRUE(checker.is_allowed("write"));
    EXPECT_TRUE(checker.is_allowed("edit"));
    EXPECT_TRUE(checker.is_allowed("exec"));
    EXPECT_FALSE(checker.is_allowed("message"));
}
