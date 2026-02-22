#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <mutex>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "quantclaw/core/content_block.hpp"

namespace quantclaw {

// --- Usage Info ---

struct UsageInfo {
    int input_tokens = 0;
    int output_tokens = 0;

    nlohmann::json to_json() const {
        return {{"inputTokens", input_tokens}, {"outputTokens", output_tokens}};
    }
    static UsageInfo from_json(const nlohmann::json& j) {
        UsageInfo u;
        u.input_tokens = j.value("inputTokens", 0);
        u.output_tokens = j.value("outputTokens", 0);
        return u;
    }
};

// --- Session Message (JSONL line format) ---

struct SessionMessage {
    std::string role;  // "user" | "assistant" | "system" | "tool"
    std::vector<ContentBlock> content;
    std::string timestamp;  // ISO 8601
    std::optional<UsageInfo> usage;

    nlohmann::json to_jsonl() const;
    static SessionMessage from_jsonl(const nlohmann::json& j);
};

// --- Session Info (sessions.json entry) ---

struct SessionInfo {
    std::string session_key;
    std::string session_id;
    std::string updated_at;
    std::string created_at;
    std::string display_name;
    std::string channel;
};

// --- Session Handle ---

struct SessionHandle {
    std::string session_key;
    std::string session_id;
    std::filesystem::path transcript_path;
};

// --- Session Manager ---

class SessionManager {
public:
    SessionManager(const std::filesystem::path& sessions_dir,
                   std::shared_ptr<spdlog::logger> logger);

    // Get or create a session by key
    SessionHandle get_or_create(const std::string& session_key,
                                const std::string& display_name = "",
                                const std::string& channel = "cli");

    // Append a message to the session transcript
    void append_message(const std::string& session_key,
                        const std::string& role,
                        const std::string& text_content,
                        const std::optional<UsageInfo>& usage = std::nullopt);

    // Append a full SessionMessage
    void append_message(const std::string& session_key, const SessionMessage& msg);

    // Get session history
    std::vector<SessionMessage> get_history(const std::string& session_key,
                                            int max_messages = -1) const;

    // List all sessions
    std::vector<SessionInfo> list_sessions() const;

    // Delete a session entirely
    void delete_session(const std::string& session_key);

    // Reset a session (archive old, create new session ID)
    void reset_session(const std::string& session_key);

    // Update display name
    void update_display_name(const std::string& session_key, const std::string& name);

    // Persistence
    void save_store();
    void load_store();

private:
    std::filesystem::path sessions_dir_;
    std::shared_ptr<spdlog::logger> logger_;
    mutable std::mutex mutex_;

    // session_key -> SessionInfo
    std::unordered_map<std::string, SessionInfo> store_;

    std::string generate_session_id() const;
    std::string get_timestamp() const;
    std::filesystem::path transcript_path(const std::string& session_id) const;
};

} // namespace quantclaw
