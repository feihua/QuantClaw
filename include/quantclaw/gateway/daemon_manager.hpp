#pragma once

#include <string>
#include <memory>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace quantclaw::gateway {

class DaemonManager {
public:
    explicit DaemonManager(std::shared_ptr<spdlog::logger> logger);

    // Service lifecycle
    int install(int port = 18789);
    int uninstall();
    int start();
    int stop();
    int restart();
    int status();

    // PID file management
    bool is_running() const;
    int get_pid() const;

private:
    std::shared_ptr<spdlog::logger> logger_;
    std::filesystem::path home_dir_;
    std::filesystem::path pid_file_;
    std::filesystem::path log_file_;

    // Platform-specific implementations
    int install_systemd(int port);
    int install_launchd(int port);
    int start_systemd();
    int start_launchd();
    int stop_systemd();
    int stop_launchd();
    int status_systemd();
    int status_launchd();

    void write_pid(int pid);
    void remove_pid();
    std::string get_executable_path() const;
};

} // namespace quantclaw::gateway
