#include <gtest/gtest.h>
#include "quantclaw/mcp/mcp_tool_manager.hpp"
#include "quantclaw/tools/tool_registry.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

class MCPToolManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto null_sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        logger_ = std::make_shared<spdlog::logger>("test", null_sink);
    }

    std::shared_ptr<spdlog::logger> logger_;
};

// --- Qualified name construction ---

TEST_F(MCPToolManagerTest, MakeQualifiedName) {
    auto name = quantclaw::mcp::MCPToolManager::make_qualified_name("code-tools", "lint");
    EXPECT_EQ(name, "mcp__code-tools__lint");
}

TEST_F(MCPToolManagerTest, MakeQualifiedNameSpecialChars) {
    auto name = quantclaw::mcp::MCPToolManager::make_qualified_name("my-server", "run_tests");
    EXPECT_EQ(name, "mcp__my-server__run_tests");
}

// --- Empty config ---

TEST_F(MCPToolManagerTest, EmptyConfigNoTools) {
    quantclaw::mcp::MCPToolManager manager(logger_);
    quantclaw::MCPConfig config;  // No servers

    manager.discover_tools(config);
    EXPECT_EQ(manager.tool_count(), 0u);
}

// --- Name resolution ---

TEST_F(MCPToolManagerTest, IsExternalToolFalseForUnknown) {
    quantclaw::mcp::MCPToolManager manager(logger_);
    EXPECT_FALSE(manager.is_external_tool("read"));
    EXPECT_FALSE(manager.is_external_tool("mcp__unknown__tool"));
}

TEST_F(MCPToolManagerTest, GetServerNameEmptyForUnknown) {
    quantclaw::mcp::MCPToolManager manager(logger_);
    EXPECT_EQ(manager.get_server_name("mcp__unknown__tool"), "");
}

TEST_F(MCPToolManagerTest, GetOriginalToolNameEmptyForUnknown) {
    quantclaw::mcp::MCPToolManager manager(logger_);
    EXPECT_EQ(manager.get_original_tool_name("mcp__unknown__tool"), "");
}

// --- Discover tools with invalid server (graceful error) ---

TEST_F(MCPToolManagerTest, DiscoverToolsInvalidServerGraceful) {
    quantclaw::mcp::MCPToolManager manager(logger_);
    quantclaw::MCPConfig config;
    quantclaw::MCPServerConfig server;
    server.name = "bad-server";
    server.url = "http://127.0.0.1:1";  // Should fail to connect
    server.timeout = 1;
    config.servers.push_back(server);

    // Should not throw, just log error
    EXPECT_NO_THROW(manager.discover_tools(config));
    EXPECT_EQ(manager.tool_count(), 0u);
}

// --- Discover tools skips empty name/url ---

TEST_F(MCPToolManagerTest, DiscoverToolsSkipsEmptyNameOrUrl) {
    quantclaw::mcp::MCPToolManager manager(logger_);
    quantclaw::MCPConfig config;

    quantclaw::MCPServerConfig s1;
    s1.name = "";
    s1.url = "http://localhost:9090";
    config.servers.push_back(s1);

    quantclaw::MCPServerConfig s2;
    s2.name = "valid";
    s2.url = "";
    config.servers.push_back(s2);

    EXPECT_NO_THROW(manager.discover_tools(config));
    EXPECT_EQ(manager.tool_count(), 0u);
}

// --- Register into ToolRegistry ---

TEST_F(MCPToolManagerTest, RegisterIntoEmptyManager) {
    quantclaw::mcp::MCPToolManager manager(logger_);
    quantclaw::ToolRegistry registry(logger_);
    registry.register_builtin_tools();

    auto schemas_before = registry.get_tool_schemas();
    manager.register_into(registry);
    auto schemas_after = registry.get_tool_schemas();

    // No MCP tools discovered, so count should not change
    EXPECT_EQ(schemas_before.size(), schemas_after.size());
}

// --- Execute tool with unknown name ---

TEST_F(MCPToolManagerTest, ExecuteToolUnknownThrows) {
    quantclaw::mcp::MCPToolManager manager(logger_);
    EXPECT_THROW(manager.execute_tool("mcp__unknown__tool", {}), std::runtime_error);
}
