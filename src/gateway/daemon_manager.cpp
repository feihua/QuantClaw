#include "quantclaw/gateway/daemon_manager.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <csignal>

namespace quantclaw::gateway {

DaemonManager::DaemonManager(std::shared_ptr<spdlog::logger> logger)
    : logger_(logger) {
    const char* home = std::getenv("HOME");
    if (home) {
        home_dir_ = std::filesystem::path(home) / ".quantclaw";
    } else {
        home_dir_ = std::filesystem::path("/tmp") / ".quantclaw";
    }
    pid_file_ = home_dir_ / "gateway.pid";
    log_file_ = home_dir_ / "logs" / "gateway.log";

    std::filesystem::create_directories(home_dir_ / "logs");
}

int DaemonManager::install(int port) {
#ifdef __APPLE__
    return install_launchd(port);
#else
    return install_systemd(port);
#endif
}

int DaemonManager::uninstall() {
    stop();

#ifdef __APPLE__
    auto plist_path = std::filesystem::path(std::getenv("HOME")) /
                      "Library/LaunchAgents/ai.quantclaw.gateway.plist";
    if (std::filesystem::exists(plist_path)) {
        std::filesystem::remove(plist_path);
        logger_->info("Removed launchd plist");
    }
#else
    auto service_path = std::filesystem::path(std::getenv("HOME")) /
                        ".config/systemd/user/quantclaw-gateway.service";
    if (std::filesystem::exists(service_path)) {
        [[maybe_unused]] int r1 = std::system("systemctl --user disable quantclaw-gateway 2>/dev/null");
        std::filesystem::remove(service_path);
        [[maybe_unused]] int r2 = std::system("systemctl --user daemon-reload 2>/dev/null");
        logger_->info("Removed systemd service");
    }
#endif

    return 0;
}

int DaemonManager::start() {
#ifdef __APPLE__
    return start_launchd();
#else
    return start_systemd();
#endif
}

int DaemonManager::stop() {
#ifdef __APPLE__
    return stop_launchd();
#else
    return stop_systemd();
#endif
}

int DaemonManager::restart() {
    stop();
    return start();
}

int DaemonManager::status() {
#ifdef __APPLE__
    return status_launchd();
#else
    return status_systemd();
#endif
}

bool DaemonManager::is_running() const {
    int pid = get_pid();
    if (pid <= 0) return false;

    // Check if process exists
    if (kill(pid, 0) == 0) {
        return true;
    }
    return false;
}

int DaemonManager::get_pid() const {
    if (!std::filesystem::exists(pid_file_)) return -1;

    std::ifstream file(pid_file_);
    int pid = -1;
    file >> pid;
    return pid;
}

// --- systemd ---

int DaemonManager::install_systemd(int port) {
    auto service_dir = std::filesystem::path(std::getenv("HOME")) /
                       ".config/systemd/user";
    std::filesystem::create_directories(service_dir);

    auto service_path = service_dir / "quantclaw-gateway.service";
    std::string exe_path = get_executable_path();

    std::ofstream file(service_path);
    file << "[Unit]\n"
         << "Description=QuantClaw Gateway\n"
         << "After=network.target\n\n"
         << "[Service]\n"
         << "Type=simple\n"
         << "ExecStart=" << exe_path << " gateway --port " << port << "\n"
         << "Restart=on-failure\n"
         << "RestartSec=5\n"
         << "StandardOutput=append:" << log_file_.string() << "\n"
         << "StandardError=append:" << log_file_.string() << "\n\n"
         << "[Install]\n"
         << "WantedBy=default.target\n";
    file.close();

    [[maybe_unused]] int r1 = std::system("systemctl --user daemon-reload");
    [[maybe_unused]] int r2 = std::system("systemctl --user enable quantclaw-gateway");

    logger_->info("Installed systemd service at {}", service_path.string());
    return 0;
}

int DaemonManager::start_systemd() {
    int ret = std::system("systemctl --user start quantclaw-gateway");
    if (ret == 0) {
        logger_->info("Gateway started via systemd");
    } else {
        logger_->error("Failed to start gateway via systemd");
    }
    return ret;
}

int DaemonManager::stop_systemd() {
    int ret = std::system("systemctl --user stop quantclaw-gateway");
    if (ret == 0) {
        logger_->info("Gateway stopped via systemd");
        remove_pid();
    }
    return ret;
}

int DaemonManager::status_systemd() {
    int ret = std::system("systemctl --user status quantclaw-gateway --no-pager");
    return ret;
}

// --- launchd ---

int DaemonManager::install_launchd(int port) {
    auto agents_dir = std::filesystem::path(std::getenv("HOME")) /
                      "Library/LaunchAgents";
    std::filesystem::create_directories(agents_dir);

    auto plist_path = agents_dir / "ai.quantclaw.gateway.plist";
    std::string exe_path = get_executable_path();

    std::ofstream file(plist_path);
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
         << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
         << "<plist version=\"1.0\">\n"
         << "<dict>\n"
         << "  <key>Label</key>\n"
         << "  <string>ai.quantclaw.gateway</string>\n"
         << "  <key>ProgramArguments</key>\n"
         << "  <array>\n"
         << "    <string>" << exe_path << "</string>\n"
         << "    <string>gateway</string>\n"
         << "    <string>--port</string>\n"
         << "    <string>" << port << "</string>\n"
         << "  </array>\n"
         << "  <key>RunAtLoad</key>\n"
         << "  <true/>\n"
         << "  <key>KeepAlive</key>\n"
         << "  <true/>\n"
         << "  <key>StandardOutPath</key>\n"
         << "  <string>" << log_file_.string() << "</string>\n"
         << "  <key>StandardErrorPath</key>\n"
         << "  <string>" << log_file_.string() << "</string>\n"
         << "</dict>\n"
         << "</plist>\n";
    file.close();

    logger_->info("Installed launchd plist at {}", plist_path.string());
    return 0;
}

int DaemonManager::start_launchd() {
    auto plist_path = std::filesystem::path(std::getenv("HOME")) /
                      "Library/LaunchAgents/ai.quantclaw.gateway.plist";
    std::string cmd = "launchctl load " + plist_path.string();
    int ret = std::system(cmd.c_str());
    if (ret == 0) {
        logger_->info("Gateway started via launchd");
    }
    return ret;
}

int DaemonManager::stop_launchd() {
    auto plist_path = std::filesystem::path(std::getenv("HOME")) /
                      "Library/LaunchAgents/ai.quantclaw.gateway.plist";
    std::string cmd = "launchctl unload " + plist_path.string();
    int ret = std::system(cmd.c_str());
    if (ret == 0) {
        logger_->info("Gateway stopped via launchd");
        remove_pid();
    }
    return ret;
}

int DaemonManager::status_launchd() {
    int ret = std::system("launchctl list | grep ai.quantclaw.gateway");
    return ret;
}

// --- PID management ---

void DaemonManager::write_pid(int pid) {
    std::ofstream file(pid_file_);
    file << pid;
    file.close();
}

void DaemonManager::remove_pid() {
    if (std::filesystem::exists(pid_file_)) {
        std::filesystem::remove(pid_file_);
    }
}

std::string DaemonManager::get_executable_path() const {
#ifdef __linux__
    char buf[4096];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        return std::string(buf);
    }
#elif defined(__APPLE__)
    char buf[4096];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        return std::string(buf);
    }
#endif
    // Fallback
    return "quantclaw";
}

} // namespace quantclaw::gateway
