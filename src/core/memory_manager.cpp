#include "quantclaw/core/memory_manager.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <thread>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace quantclaw {

MemoryManager::MemoryManager(const std::filesystem::path& workspace_path,
                             std::shared_ptr<spdlog::logger> logger)
    : workspace_path_(workspace_path), logger_(logger) {

    // Determine base dir from workspace path
    // Expected: ~/.quantclaw/agents/{agentId}/workspace
    // Or legacy: ~/.quantclaw/workspace
    if (workspace_path_.string().find("/agents/") != std::string::npos) {
        // New layout: base_dir is 3 levels up from workspace
        base_dir_ = workspace_path_.parent_path().parent_path().parent_path();
    } else {
        // Legacy layout: base_dir is one level up
        base_dir_ = workspace_path_.parent_path();
    }

    std::filesystem::create_directories(workspace_path_);
    logger_->info("MemoryManager initialized with workspace: {}",
                  workspace_path_.string());
}

MemoryManager::~MemoryManager() {
    stop_file_watcher();
}

void MemoryManager::load_workspace_files() {
    logger_->info("Loading workspace files from: {}", workspace_path_.string());

    for (const auto& name : {"SOUL.md", "USER.md", "MEMORY.md", "AGENTS.md", "TOOLS.md"}) {
        try {
            auto content = read_identity_file(name);
            if (!content.empty()) {
                logger_->debug("Loaded {} ({} bytes)", name, content.size());
            }
        } catch (const std::exception& e) {
            logger_->debug("No {} found: {}", name, e.what());
        }
    }

    // Load daily memory files
    auto memory_dir = workspace_path_ / "memory";
    if (std::filesystem::exists(memory_dir)) {
        int loaded_count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(memory_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".md") {
                try {
                    auto content = read_file_content(entry.path());
                    loaded_count++;
                } catch (const std::exception& e) {
                    logger_->warn("Failed to load memory file {}: {}",
                                  entry.path().filename().string(), e.what());
                }
            }
        }
        logger_->info("Loaded {} daily memory files", loaded_count);
    }

    logger_->info("Workspace files loaded successfully");
}

std::string MemoryManager::read_identity_file(const std::string& filename) {
    auto filepath = workspace_path_ / filename;
    return read_file_content(filepath);
}

std::string MemoryManager::read_agents_file() {
    return read_identity_file("AGENTS.md");
}

std::string MemoryManager::read_tools_file() {
    return read_identity_file("TOOLS.md");
}

std::vector<std::string> MemoryManager::search_memory(const std::string& query) {
    std::vector<std::string> results;

    std::vector<std::string> identity_files = {"SOUL.md", "USER.md", "MEMORY.md", "AGENTS.md", "TOOLS.md"};
    for (const auto& filename : identity_files) {
        try {
            auto content = read_identity_file(filename);
            if (content.find(query) != std::string::npos) {
                results.push_back("File: " + filename + "\nContent: " + content);
            }
        } catch (const std::exception&) {
            // Skip files that can't be read
        }
    }

    auto memory_dir = workspace_path_ / "memory";
    if (std::filesystem::exists(memory_dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(memory_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".md") {
                try {
                    auto content = read_file_content(entry.path());
                    if (content.find(query) != std::string::npos) {
                        results.push_back("File: " + entry.path().string() + "\nContent: " + content);
                    }
                } catch (const std::exception&) {
                }
            }
        }
    }

    return results;
}

void MemoryManager::save_daily_memory(const std::string& content) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time_t);
#else
    localtime_r(&time_t, &tm);
#endif

    std::ostringstream date_stream;
    date_stream << std::put_time(&tm, "%Y-%m-%d");
    auto date_str = date_stream.str();

    auto memory_dir = workspace_path_ / "memory";
    std::filesystem::create_directories(memory_dir);

    std::ostringstream entry_stream;
    entry_stream << "## " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "\n";
    entry_stream << content << "\n";
    auto entry_content = entry_stream.str();

    auto memory_file = memory_dir / (date_str + ".md");
    std::ofstream file(memory_file, std::ios::app);
    if (file.is_open()) {
        file << entry_content;
        file.close();
        logger_->debug("Saved memory entry to {}", memory_file.string());
    } else {
        logger_->error("Failed to save memory entry to {}", memory_file.string());
        throw std::runtime_error("Failed to write to memory file: " + memory_file.string());
    }
}

void MemoryManager::start_file_watcher() {
    if (watching_) return;
    watching_ = true;

    // Record current mtimes
    for (const auto& name : {"SOUL.md", "USER.md", "MEMORY.md", "AGENTS.md", "TOOLS.md"}) {
        auto path = workspace_path_ / name;
        if (std::filesystem::exists(path)) {
            file_mtimes_[name] = std::filesystem::last_write_time(path);
        }
    }

    watcher_thread_ = std::make_unique<std::thread>([this]() {
        while (watching_) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            if (!watching_) break;

            for (const auto& name : {"SOUL.md", "USER.md", "MEMORY.md", "AGENTS.md", "TOOLS.md"}) {
                auto path = workspace_path_ / name;
                if (!std::filesystem::exists(path)) continue;
                auto mtime = std::filesystem::last_write_time(path);
                auto it = file_mtimes_.find(name);
                if (it == file_mtimes_.end() || it->second != mtime) {
                    file_mtimes_[name] = mtime;
                    logger_->info("File changed: {}", name);
                    if (change_callback_) {
                        change_callback_(name);
                    }
                }
            }
        }
    });

    logger_->info("File watcher started for workspace: {}", workspace_path_.string());
}

void MemoryManager::stop_file_watcher() {
    if (!watching_) return;
    watching_ = false;
    if (watcher_thread_ && watcher_thread_->joinable()) {
        watcher_thread_->join();
    }
    watcher_thread_.reset();
    logger_->info("File watcher stopped");
}

void MemoryManager::set_file_change_callback(FileChangeCallback cb) {
    change_callback_ = std::move(cb);
}

const std::filesystem::path& MemoryManager::get_workspace_path() const {
    return workspace_path_;
}

void MemoryManager::set_agent_workspace(const std::string& agent_id) {
    agent_id_ = agent_id;
    workspace_path_ = base_dir_ / "agents" / agent_id / "workspace";
    std::filesystem::create_directories(workspace_path_);
    logger_->info("Set agent workspace: {}", workspace_path_.string());
}

std::filesystem::path MemoryManager::get_base_dir() const {
    return base_dir_;
}

std::filesystem::path MemoryManager::get_sessions_dir(const std::string& agent_id) const {
    return base_dir_ / "agents" / agent_id / "sessions";
}

bool MemoryManager::is_memory_file(const std::filesystem::path& filepath) const {
    auto filename = filepath.filename().string();

    if (filename == "SOUL.md" || filename == "USER.md" || filename == "MEMORY.md" ||
        filename == "AGENTS.md" || filename == "TOOLS.md") {
        return true;
    }

    if (filepath.parent_path().filename() == "memory" &&
        filepath.extension() == ".md") {
        return true;
    }

    return false;
}

std::string MemoryManager::read_file_content(const std::filesystem::path& filepath) const {
    if (!std::filesystem::exists(filepath)) {
        throw std::runtime_error("File not found: " + filepath.string());
    }

    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath.string());
    }

    std::ostringstream content;
    content << file.rdbuf();
    file.close();

    return content.str();
}

void MemoryManager::write_file_content(const std::filesystem::path& filepath,
                                        const std::string& content) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to write file: " + filepath.string());
    }

    file << content;
    file.close();
}

} // namespace quantclaw
