#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <atomic>
#include <nlohmann/json.hpp>
#include "quantclaw/providers/llm_provider.hpp"
#include "quantclaw/config.hpp"
#include <spdlog/spdlog.h>

namespace quantclaw {

class MemoryManager;
class SkillLoader;
class ToolRegistry;

// --- Agent Event (for streaming) ---

struct AgentEvent {
    std::string type;  // "text_delta" | "tool_use" | "tool_result" | "message_end"
    nlohmann::json data;
};

using AgentEventCallback = std::function<void(const AgentEvent&)>;

// --- Agent Loop ---

class AgentLoop {
public:
    AgentLoop(std::shared_ptr<MemoryManager> memory_manager,
              std::shared_ptr<SkillLoader> skill_loader,
              std::shared_ptr<ToolRegistry> tool_registry,
              std::shared_ptr<LLMProvider> llm_provider,
              const AgentConfig& agent_config,
              std::shared_ptr<spdlog::logger> logger);

    // Process a message with externally-provided history and system prompt
    // Returns all new messages generated during the turn (assistant + tool_result)
    std::vector<Message> process_message(const std::string& message,
                                         const std::vector<Message>& history,
                                         const std::string& system_prompt);

    // Streaming version — returns all new messages generated during the turn
    std::vector<Message> process_message_stream(const std::string& message,
                                                 const std::vector<Message>& history,
                                                 const std::string& system_prompt,
                                                 AgentEventCallback callback);

    // Stop the current agent turn
    void stop();

    // Set max iterations
    void set_max_iterations(int max) { max_iterations_ = max; }

    // Update agent config (for hot-reload)
    void set_config(const AgentConfig& config);

    // Get current config (for testing)
    const AgentConfig& get_config() const { return agent_config_; }

private:
    std::vector<std::string> handle_tool_calls(
        const std::vector<nlohmann::json>& tool_calls);

    std::shared_ptr<MemoryManager> memory_manager_;
    std::shared_ptr<SkillLoader> skill_loader_;
    std::shared_ptr<ToolRegistry> tool_registry_;
    std::shared_ptr<LLMProvider> llm_provider_;
    std::shared_ptr<spdlog::logger> logger_;
    AgentConfig agent_config_;
    std::atomic<bool> stop_requested_{false};
    int max_iterations_ = 15;
};

} // namespace quantclaw
