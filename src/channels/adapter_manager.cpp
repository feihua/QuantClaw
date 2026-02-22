#include "quantclaw/channels/adapter_manager.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>
#include <thread>

namespace quantclaw {

ChannelAdapterManager::ChannelAdapterManager(
    int gateway_port,
    const std::string& auth_token,
    const std::unordered_map<std::string, ChannelConfig>& channels,
    std::shared_ptr<spdlog::logger> logger)
    : gateway_port_(gateway_port)
    , auth_token_(auth_token)
    , channels_(channels)
    , logger_(logger) {
}

ChannelAdapterManager::~ChannelAdapterManager() {
    stop();
}

std::string ChannelAdapterManager::find_adapter_script(const std::string& channel_name) const {
    // Search order:
    // 1. ~/.quantclaw/adapters/<name>.ts
    // 2. Bundled adapters relative to executable
    // 3. Source tree (development)

    const char* home = std::getenv("HOME");
    std::string home_str = home ? home : "/tmp";

    std::vector<std::string> search_paths = {
        home_str + "/.quantclaw/adapters/" + channel_name + ".ts",
    };

    // Relative to the executable's directory
    try {
        auto exe_path = std::filesystem::read_symlink("/proc/self/exe");
        auto exe_dir = exe_path.parent_path();
        search_paths.push_back((exe_dir / "adapters" / (channel_name + ".ts")).string());
        search_paths.push_back((exe_dir.parent_path() / "adapters" / (channel_name + ".ts")).string());
    } catch (...) {}

    for (const auto& path : search_paths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }

    return "";
}

bool ChannelAdapterManager::launch_adapter(AdapterProcess& adapter, const ChannelConfig& config) {
    // Pass the full raw channel config to the adapter
    std::string gateway_url = "ws://127.0.0.1:" + std::to_string(gateway_port_);
    nlohmann::json config_json = config.raw;
    // Ensure token is present (may come from top-level or platform-specific field)
    if (!config.token.empty() && !config_json.contains("token")) {
        config_json["token"] = config.token;
    }

    pid_t pid = fork();
    if (pid < 0) {
        logger_->error("Failed to fork adapter process for {}: {}", adapter.name, strerror(errno));
        return false;
    }

    if (pid == 0) {
        // Child process — set env vars and exec
        setenv("QUANTCLAW_GATEWAY_URL", gateway_url.c_str(), 1);
        setenv("QUANTCLAW_AUTH_TOKEN", auth_token_.c_str(), 1);
        setenv("QUANTCLAW_CHANNEL_NAME", adapter.name.c_str(), 1);
        setenv("QUANTCLAW_CHANNEL_CONFIG", config_json.dump().c_str(), 1);

        // Set platform-specific env var for convenience (e.g. DISCORD_BOT_TOKEN)
        if (!config.token.empty()) {
            std::string env_name = adapter.name;
            for (auto& c : env_name) c = static_cast<char>(std::toupper(c));
            setenv((env_name + "_BOT_TOKEN").c_str(), config.token.c_str(), 1);
        }

        // cd into the adapter script's directory so node_modules resolves
        auto script_dir = std::filesystem::path(adapter.script_path).parent_path().string();
        if (chdir(script_dir.c_str()) != 0) {
            _exit(1);
        }

        // Run via npx tsx (TypeScript execution)
        execlp("npx", "npx", "tsx", adapter.script_path.c_str(), nullptr);

        // Fallback: try ts-node
        execlp("npx", "npx", "ts-node", "--esm", adapter.script_path.c_str(), nullptr);

        // Last resort: node (if pre-compiled to .js)
        std::string js_path = adapter.script_path;
        auto dot = js_path.rfind(".ts");
        if (dot != std::string::npos) {
            js_path.replace(dot, 3, ".js");
        }
        execlp("node", "node", js_path.c_str(), nullptr);

        _exit(1);
    }

    // Parent process
    adapter.pid = pid;
    adapter.running = true;
    logger_->info("Launched adapter '{}' (PID: {}, script: {})",
                  adapter.name, pid, adapter.script_path);
    return true;
}

void ChannelAdapterManager::kill_adapter(AdapterProcess& adapter) {
    if (!adapter.running || adapter.pid <= 0) return;

    logger_->info("Stopping adapter '{}' (PID: {})", adapter.name, adapter.pid);

    // Send SIGTERM first
    kill(adapter.pid, SIGTERM);

    // Wait up to 5 seconds for graceful shutdown
    for (int i = 0; i < 50; ++i) {
        int status;
        pid_t result = waitpid(adapter.pid, &status, WNOHANG);
        if (result != 0) {
            adapter.running = false;
            adapter.pid = 0;
            logger_->info("Adapter '{}' stopped", adapter.name);
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Force kill
    kill(adapter.pid, SIGKILL);
    waitpid(adapter.pid, nullptr, 0);
    adapter.running = false;
    adapter.pid = 0;
    logger_->warn("Adapter '{}' force-killed", adapter.name);
}

void ChannelAdapterManager::start() {
    if (running_) return;

    for (const auto& [name, config] : channels_) {
        if (!config.enabled) {
            logger_->debug("Channel '{}' disabled, skipping", name);
            continue;
        }

        std::string script = find_adapter_script(name);
        if (script.empty()) {
            logger_->warn("No adapter script found for channel '{}', skipping", name);
            continue;
        }

        AdapterProcess proc;
        proc.name = name;
        proc.script_path = script;

        if (launch_adapter(proc, config)) {
            adapters_.push_back(std::move(proc));
        }
    }

    if (!adapters_.empty()) {
        running_ = true;
        monitor_thread_ = std::make_unique<std::thread>(&ChannelAdapterManager::monitor_loop, this);
        logger_->info("ChannelAdapterManager started with {} adapter(s)", adapters_.size());
    } else {
        logger_->info("No channel adapters to start");
    }
}

void ChannelAdapterManager::stop() {
    running_ = false;

    for (auto& adapter : adapters_) {
        kill_adapter(adapter);
    }
    adapters_.clear();

    if (monitor_thread_ && monitor_thread_->joinable()) {
        monitor_thread_->join();
    }
    monitor_thread_.reset();
}

std::vector<std::string> ChannelAdapterManager::running_adapters() const {
    std::vector<std::string> names;
    for (const auto& adapter : adapters_) {
        if (adapter.running) {
            names.push_back(adapter.name);
        }
    }
    return names;
}

void ChannelAdapterManager::monitor_loop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        if (!running_) break;

        for (auto& adapter : adapters_) {
            if (!adapter.running) continue;

            int status;
            pid_t result = waitpid(adapter.pid, &status, WNOHANG);
            if (result > 0) {
                // Process exited
                if (WIFEXITED(status)) {
                    logger_->warn("Adapter '{}' exited with code {}", adapter.name, WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    logger_->warn("Adapter '{}' killed by signal {}", adapter.name, WTERMSIG(status));
                }

                adapter.running = false;
                adapter.pid = 0;

                // Auto-restart if manager is still running
                if (running_) {
                    logger_->info("Restarting adapter '{}'...", adapter.name);
                    auto it = channels_.find(adapter.name);
                    if (it != channels_.end()) {
                        std::this_thread::sleep_for(std::chrono::seconds(3));
                        launch_adapter(adapter, it->second);
                    }
                }
            }
        }
    }
}

} // namespace quantclaw
