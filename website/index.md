---
layout: home

hero:
  name: "QuantClaw"
  text: "High-Performance AI Agent Framework"
  tagline: "C++17 implementation of OpenClaw with persistent memory, browser control, and plugin ecosystem"
  image:
    src: /logo.svg
    alt: QuantClaw
  actions:
    - theme: brand
      text: Get Started
      link: /guide/getting-started
    - theme: alt
      text: View on GitHub
      link: https://github.com/QuantClaw/quantclaw

features:
  - icon: ⚡
    title: "High Performance"
    details: "C++17 native implementation delivering exceptional speed and low resource footprint for AI agent operations"

  - icon: 🧠
    title: "Persistent Memory"
    details: "Advanced context management with automatic compaction, memory search, and intelligent context pruning"

  - icon: 🌐
    title: "Browser Control"
    details: "Full Chrome DevTools Protocol support for web automation, scraping, and intelligent interaction"

  - icon: 🔌
    title: "Plugin Ecosystem"
    details: "Extensible architecture with Node.js sidecar runtime for custom skills, hooks, and integrations"

  - icon: 🛡️
    title: "Enterprise Security"
    details: "Comprehensive RBAC, sandboxed execution, tool permissions, and multi-level authorization controls"

  - icon: 📱
    title: "Multi-Platform"
    details: "Unified codebase running seamlessly on Ubuntu and Windows with Docker containerization support"

  - icon: 🔄
    title: "Model Agnostic"
    details: "Support for 7+ LLM providers with automatic failover, fallback chains, and profile-based selection"

  - icon: 💬
    title: "Multi-Channel"
    details: "Native integration with Discord, Slack, Telegram, WhatsApp, Email, and custom HTTP endpoints"

  - icon: 🎯
    title: "OpenClaw Compatible"
    details: "100% compatible CLI, RPC protocols, and configuration format with the TypeScript original"
---

<script setup>
import { ref } from 'vue'

const testimonials = [
  {
    text: "QuantClaw brings the power of OpenClaw to production environments with C++ performance.",
    author: "AI Developer",
    role: "Tech Lead"
  },
  {
    text: "The memory management and context optimization features are industry-leading.",
    author: "Research Engineer",
    role: "AI/ML"
  },
  {
    text: "Seamless integration with existing OpenClaw skills and plugins.",
    author: "DevOps Engineer",
    role: "Platform Team"
  }
]
</script>

## Why QuantClaw?

**QuantClaw** is a high-performance C++17 reimplement of OpenClaw, the AI agent framework that actually does things. It combines the full feature set of OpenClaw with native performance benefits:

- **Production Ready**: Industry-grade error handling, monitoring, and resilience
- **Scalable**: Optimized for high-throughput, multi-agent deployments
- **Compatible**: 100% CLI and RPC compatibility with OpenClaw
- **Extensible**: Plugin ecosystem with Node.js sidecar for custom capabilities

## Quick Comparison

| Feature | QuantClaw | OpenClaw |
|---------|-----------|----------|
| Language | C++17 | TypeScript |
| Performance | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ |
| Memory Usage | Low | Medium |
| Startup Time | Fast | Standard |
| Deployment | Docker/Binary | Node.js |
| Plugin Support | ✅ Sidecar | ✅ Native |
| CLI Compatibility | 100% | - |

## Key Capabilities

### Intelligent Conversation
- Context-aware multi-turn dialogue
- Automatic memory compaction for efficiency
- Thinking mode for complex reasoning
- Session management with history

### Persistent Memory
- Automatic memory search and retrieval
- User-specific knowledge bases
- Workspace file management
- Smart context pruning

### Browser Automation
- Full Chrome DevTools Protocol support
- Page scraping and interaction
- Visual understanding
- Cookie and session management

### System Integration
- Command execution with safety controls
- File system access with sandboxing
- Process management
- Environment variable support

### Extensible Architecture
- Custom skill development
- Hook system for lifecycle events
- Multi-provider LLM support
- Custom channel integration

## Getting Started

The fastest way to get started with QuantClaw:

```bash
# Clone the repository
git clone https://github.com/QuantClaw/quantclaw.git
cd quantclaw

# Build from source
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)

# Run the agent
./quantclaw agent --id=main
```

For detailed installation instructions, see [Getting Started](/guide/getting-started).

## Community & Support

- **GitHub**: [QuantClaw/quantclaw](https://github.com/QuantClaw/quantclaw)
- **Issues**: [Report bugs and request features](https://github.com/QuantClaw/quantclaw/issues)
- **Discussions**: [Community discussions](https://github.com/QuantClaw/quantclaw/discussions)

## License

QuantClaw is released under the [MIT License](https://github.com/QuantClaw/quantclaw/blob/main/LICENSE). The original OpenClaw project is also MIT licensed, allowing free commercial and open-source use.

---

**Made with ❤️ by the QuantClaw team. Inspired by [OpenClaw](https://openclaw.ai).**
