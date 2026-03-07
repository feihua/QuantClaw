# Getting Started with QuantClaw

Welcome to QuantClaw! This guide will help you get up and running in just a few minutes.

## What is QuantClaw?

QuantClaw is a high-performance C++17 implementation of OpenClaw, an AI agent framework designed to run locally on your machine. It can execute commands, control browsers, manage files, and integrate with various chat platforms.

## Prerequisites

Before you start, make sure you have:

- **Linux (Ubuntu 20.04+)** or **Windows 10+ (WSL2)**
- **C++17 compatible compiler** (GCC 7+, Clang 6+, MSVC 2017+)
- **CMake 3.15+**
- **Node.js 16+** (for plugin support)
- **Docker** (optional, for containerized deployment)

## Installation Methods

### Method 1: Quick Docker Setup

The fastest way to get started:

```bash
docker run -d \
  --name quantclaw \
  -p 8000:8000 \
  -v ~/.quantclaw:/root/.quantclaw \
  ghcr.io/quantclaw/quantclaw:latest
```

### Method 2: Build from Source

Clone and build QuantClaw:

```bash
# Clone the repository
git clone https://github.com/QuantClaw/quantclaw.git
cd quantclaw

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build . -j$(nproc)

# Verify installation
./quantclaw_tests
```

### Method 3: Pre-built Binaries

Download pre-built binaries for your platform from [GitHub Releases](https://github.com/QuantClaw/quantclaw/releases).

```bash
# Linux
wget https://github.com/QuantClaw/quantclaw/releases/download/v1.0.0/quantclaw-linux-x64.tar.gz
tar xzf quantclaw-linux-x64.tar.gz
./quantclaw --version

# Windows
# Download and extract the .zip from releases
```

## Initial Setup

Once installed, initialize QuantClaw:

```bash
# Onboard configuration
quantclaw onboard --quick

# This will:
# - Create configuration directory (~/.quantclaw)
# - Initialize workspace structure
# - Prompt for API keys (OpenAI, Anthropic, etc.)
```

You'll be asked for:
- **LLM Provider**: OpenAI, Anthropic, or other providers
- **API Key**: Your authentication credentials
- **Model Selection**: Default model to use (e.g., GPT-4, Claude 3)

## Your First Agent

Start your first QuantClaw agent:

```bash
# Start the agent in foreground
quantclaw agent --id=main

# Or start the gateway daemon
quantclaw gateway run
```

The agent will:
- Load your configuration
- Initialize memory and context
- Start listening for commands
- Display a prompt or web interface

## Using the Web Interface

QuantClaw includes a built-in web dashboard:

```bash
# Start the gateway (if not already running)
quantclaw gateway run &

# Open in browser
open http://localhost:8000
```

Features:
- 💬 Chat interface
- 📊 Memory and context viewer
- ⚙️ Configuration management
- 📈 Usage and token tracking
- 🔧 Plugin management

## Command Line Usage

Try your first commands:

```bash
# Simple question
quantclaw run "What is the weather today?"

# Run without session (eval mode)
quantclaw run --no-session "2 + 2"

# Access your workspace files
quantclaw file list

# Check status
quantclaw status
```

## Configuration

The main configuration file is `~/.quantclaw/config.json`:

```json
{
  "agents": {
    "main": {
      "models": {
        "default": "claude-3-5-sonnet-20241022",
        "reasoning": "claude-3-7-opus-20250219"
      },
      "providers": {
        "default": "anthropic"
      },
      "memory": {
        "maxTokens": 100000,
        "compactionThreshold": 80000
      }
    }
  }
}
```

## Next Steps

- 📖 [Read the full documentation](/guide/documentation)
- 🏗️ [Learn the architecture](/guide/architecture)
- 🔌 [Create your first plugin](/guide/plugins)
- 🛠️ [CLI reference](/guide/cli-reference)

## Getting Help

- **Documentation**: [Full docs](/guide/documentation)
- **GitHub Issues**: [Report bugs](https://github.com/QuantClaw/quantclaw/issues)
- **Discussions**: [Community support](https://github.com/QuantClaw/quantclaw/discussions)

## Troubleshooting

### Build Errors

If you encounter build errors:

```bash
# Clean build directory
rm -rf build
mkdir build && cd build

# Try with verbose output
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
cmake --build .
```

### Configuration Issues

Validate your configuration:

```bash
quantclaw config validate
quantclaw config schema
```

### API Key Problems

Check your API key configuration:

```bash
quantclaw status
quantclaw config get agents.main.providers.default
```

## Performance Tips

- **Memory Management**: Enable context compaction for long conversations
- **Token Budgeting**: Set `maxContextTokens` to prevent overflow
- **Model Selection**: Use faster models for quick tasks, reasoning models for complex ones
- **Plugin Optimization**: Load only necessary plugins

---

🎉 **Congratulations!** You're ready to start building with QuantClaw. Check out the [feature guide](/guide/features) to explore more capabilities.
