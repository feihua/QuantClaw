#include <gtest/gtest.h>
#include <memory>
#include <filesystem>
#include "quantclaw/security/sandbox.hpp"
#ifdef __linux__
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

class SandboxTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = std::filesystem::temp_directory_path() / "quantclaw_sandbox_test";
        std::filesystem::create_directories(test_dir_);
    }

    void TearDown() override {
        if (std::filesystem::exists(test_dir_)) {
            std::filesystem::remove_all(test_dir_);
        }
    }

    std::filesystem::path test_dir_;
};

TEST_F(SandboxTest, AllowedPathWithinWorkspace) {
    quantclaw::Sandbox sandbox(test_dir_,
        {test_dir_.string()},  // allowed
        {},                     // denied
        {},                     // allowed commands
        {}                      // denied commands
    );

    auto file_in_workspace = test_dir_ / "SOUL.md";
    EXPECT_TRUE(sandbox.is_path_allowed(file_in_workspace.string()));
}

TEST_F(SandboxTest, DeniedPathOutsideWorkspace) {
    quantclaw::Sandbox sandbox(test_dir_,
        {test_dir_.string()},  // allowed
        {},                     // denied
        {},
        {}
    );

    EXPECT_FALSE(sandbox.is_path_allowed("/etc/passwd"));
}

TEST_F(SandboxTest, ExplicitDenyOverridesAllow) {
    quantclaw::Sandbox sandbox(test_dir_,
        {"/"},                  // allow everything
        {"/etc"},               // but deny /etc
        {},
        {}
    );

    EXPECT_FALSE(sandbox.is_path_allowed("/etc/passwd"));
    EXPECT_TRUE(sandbox.is_path_allowed("/tmp/test.txt"));
}

TEST_F(SandboxTest, EmptyAllowedPathsPermitsAll) {
    quantclaw::Sandbox sandbox(test_dir_,
        {},   // no allowed paths → permit all (except denied)
        {},
        {},
        {}
    );

    EXPECT_TRUE(sandbox.is_path_allowed("/tmp/anything"));
}

TEST_F(SandboxTest, SanitizePathTraversal) {
    quantclaw::Sandbox sandbox(test_dir_, {}, {}, {}, {});

    EXPECT_THROW(sandbox.sanitize_path("../../../etc/passwd"), std::runtime_error);
}

TEST_F(SandboxTest, SanitizeNormalPath) {
    quantclaw::Sandbox sandbox(test_dir_, {}, {}, {}, {});

    auto result = sandbox.sanitize_path(test_dir_.string() + "/SOUL.md");
    EXPECT_FALSE(result.empty());
}

// --- Static validators ---

TEST_F(SandboxTest, ValidateFilePath) {
    EXPECT_TRUE(quantclaw::Sandbox::validate_file_path("/tmp/test.txt", "/tmp"));
    EXPECT_FALSE(quantclaw::Sandbox::validate_file_path("../../etc/passwd", "/tmp"));
}

TEST_F(SandboxTest, ValidateShellCommandSafe) {
    EXPECT_TRUE(quantclaw::Sandbox::validate_shell_command("ls -la"));
    EXPECT_TRUE(quantclaw::Sandbox::validate_shell_command("echo hello"));
}

TEST_F(SandboxTest, ValidateShellCommandDangerous) {
    EXPECT_FALSE(quantclaw::Sandbox::validate_shell_command("rm -rf /"));
    EXPECT_FALSE(quantclaw::Sandbox::validate_shell_command("dd if=/dev/zero of=/dev/sda"));
    EXPECT_FALSE(quantclaw::Sandbox::validate_shell_command("mkfs.ext4 /dev/sda"));
}

// --- Command filtering ---

TEST_F(SandboxTest, DenyCommandByPattern) {
    quantclaw::Sandbox sandbox(test_dir_,
        {},
        {},
        {},
        {"rm\\s+-rf"}  // denied command pattern (regex)
    );

    EXPECT_FALSE(sandbox.is_command_allowed("rm -rf /"));
    EXPECT_TRUE(sandbox.is_command_allowed("ls -la"));
}

// --- Resource limits ---

TEST_F(SandboxTest, ApplyResourceLimitsDoesNotThrow) {
#ifdef __linux__
    // Run in a child process to avoid permanently restricting the test process
    pid_t pid = fork();
    ASSERT_NE(pid, -1) << "fork() failed";
    if (pid == 0) {
        quantclaw::Sandbox::apply_resource_limits();
        _exit(0);  // No throw
    }
    int status;
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0);
#else
    EXPECT_NO_THROW(quantclaw::Sandbox::apply_resource_limits());
#endif
}

#ifdef __linux__
TEST_F(SandboxTest, ResourceLimitsAreSet) {
    // Fork a child to test resource limits without affecting the test process.
    // Hard limits can't be raised back without root privileges.
    pid_t pid = fork();
    ASSERT_NE(pid, -1) << "fork() failed";

    if (pid == 0) {
        // Child process: apply limits and verify
        quantclaw::Sandbox::apply_resource_limits();

        struct rlimit cpu_limit;
        getrlimit(RLIMIT_CPU, &cpu_limit);
        if (cpu_limit.rlim_cur != 30 || cpu_limit.rlim_max != 60) _exit(1);

        struct rlimit fsize_limit;
        getrlimit(RLIMIT_FSIZE, &fsize_limit);
        if (fsize_limit.rlim_cur != 64u * 1024 * 1024 ||
            fsize_limit.rlim_max != 128u * 1024 * 1024) _exit(2);

        struct rlimit nproc_limit;
        getrlimit(RLIMIT_NPROC, &nproc_limit);
        if (nproc_limit.rlim_cur != 32 || nproc_limit.rlim_max != 64) _exit(3);

        _exit(0);  // All checks passed
    }

    // Parent: wait for child and check exit status
    int status;
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0)
        << "Child exit code indicates resource limit mismatch";
}
#endif
