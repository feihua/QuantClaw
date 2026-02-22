#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace quantclaw {

// --- Data Structures ---

struct ChainStep {
    std::string tool_name;
    nlohmann::json arguments;  // May contain {{prev.result}}, {{steps[N].result}}
};

enum class ChainErrorPolicy { StopOnError, ContinueOnError, Retry };

struct ToolChainDef {
    std::string name;
    std::string description;
    std::vector<ChainStep> steps;
    ChainErrorPolicy error_policy = ChainErrorPolicy::StopOnError;
    int max_retries = 1;
};

struct ChainStepResult {
    int step_index;
    std::string tool_name;
    std::string result;
    std::string error;
    bool success;
};

struct ChainResult {
    std::string chain_name;
    std::vector<ChainStepResult> step_results;
    bool success;
    std::string final_result;
};

// --- Template Engine ---

class ChainTemplateEngine {
public:
    // Resolve template references in a JSON value using previous step results
    static nlohmann::json resolve(const nlohmann::json& value,
                                  const std::vector<ChainStepResult>& previous_results);

private:
    static std::string resolve_string(const std::string& tmpl,
                                      const std::vector<ChainStepResult>& previous_results);
};

// --- Executor ---

using ToolExecutorFn = std::function<std::string(const std::string&, const nlohmann::json&)>;

class ToolChainExecutor {
public:
    ToolChainExecutor(ToolExecutorFn executor, std::shared_ptr<spdlog::logger> logger);

    ChainResult execute(const ToolChainDef& chain);

    static ToolChainDef parse_chain(const nlohmann::json& j);
    static nlohmann::json result_to_json(const ChainResult& result);

private:
    ToolExecutorFn executor_;
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace quantclaw
