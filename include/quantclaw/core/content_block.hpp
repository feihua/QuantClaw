#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace quantclaw {

struct ContentBlock {
    std::string type;  // "text" | "tool_use" | "tool_result" | "thinking"
    // For text/thinking
    std::string text;
    // For tool_use
    std::string id;
    std::string name;
    nlohmann::json input;
    // For tool_result
    std::string tool_use_id;
    std::string content;

    nlohmann::json to_json() const;
    static ContentBlock from_json(const nlohmann::json& j);
    static ContentBlock make_text(const std::string& text);
    static ContentBlock make_tool_use(const std::string& id, const std::string& name, const nlohmann::json& input);
    static ContentBlock make_tool_result(const std::string& tool_use_id, const std::string& content);
};

} // namespace quantclaw
