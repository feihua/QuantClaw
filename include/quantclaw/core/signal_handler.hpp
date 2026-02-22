#pragma once

#include <functional>
#include <atomic>

namespace quantclaw {

class SignalHandler {
public:
    using ShutdownCallback = std::function<void()>;
    using ReloadCallback = std::function<void()>;

    static void install(ShutdownCallback on_shutdown, ReloadCallback on_reload = nullptr);
    static void wait_for_shutdown();
    static bool should_shutdown();

private:
    static std::atomic<bool> shutdown_requested_;
    static ShutdownCallback shutdown_callback_;
    static ReloadCallback reload_callback_;
    static void signal_handler(int signum);
};

} // namespace quantclaw
