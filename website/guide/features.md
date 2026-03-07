# Core Features

QuantClaw combines powerful AI capabilities with local-first execution and robust error handling.

## 🧠 Intelligent Conversation

Multi-turn dialogue with full context awareness:

- **Context Management**: Automatic compaction and pruning to stay within token limits
- **Thinking Mode**: Enable extended reasoning for complex problems
- **Session History**: Persistent conversation tracking and replay
- **Memory Integration**: Automatic context retrieval from knowledge bases

```bash
quantclaw run "Help me analyze this code"
```

## 💾 Persistent Memory System

Advanced memory management inspired by human cognition:

### Memory Types
- **User Memory**: Persistent user profiles and preferences
- **Agent Memory**: Long-term learned patterns and behaviors
- **Working Memory**: Current session context
- **Workspace Files**: SOUL, IDENTITY, HEARTBEAT, MEMORY, SKILL

### Memory Operations
```bash
quantclaw memory search "user preferences"
quantclaw memory get workspace/MEMORY.md
quantclaw memory append "New fact about user behavior"
```

### Automatic Context Pruning
- BM25-based relevance scoring
- Overflow compaction with 3-attempt retry
- Budget-based context management
- Thinking level auto-adjustment

## 🌐 Browser Control

Full Chrome DevTools Protocol integration:

```bash
# Launch browser and navigate
quantclaw tool browser.launch "https://example.com"

# Execute JavaScript
quantclaw tool browser.execute "{\"code\": \"document.title\"}"

# Take screenshots
quantclaw tool browser.screenshot
```

### Capabilities
- **Page Navigation**: Load and control URLs
- **DOM Interaction**: Click, type, submit forms
- **JavaScript Execution**: Run custom scripts
- **Screenshot Capture**: Visual verification
- **Cookie Management**: Session persistence
- **Network Monitoring**: Intercept requests

## 💻 System Integration

Execute commands and manage system resources:

### Command Execution
```bash
quantclaw tool bash "ls -la /home"
quantclaw tool bash "npm install package-name"
```

### File Operations
```bash
quantclaw file list
quantclaw file read workspace/config.json
quantclaw file write workspace/data.txt "content"
```

### Process Management
```bash
quantclaw tool process "start" "background_job.sh"
quantclaw tool process "list"
quantclaw tool process "kill" "job_id"
```

### Environment Access
```bash
quantclaw config get agents.main.env.API_KEY
```

## 🔌 Plugin Ecosystem

Extensible architecture with Node.js sidecar:

### Built-in Plugins
- **Bash Tools**: Safe command execution
- **Browser Control**: CDP integration
- **Web Search**: Multi-provider search (Tavily, DuckDuckGo, Perplexity)
- **Web Fetch**: HTML scraping with SSRF protection
- **Memory Search**: BM25-based knowledge retrieval
- **Cron Scheduling**: Automated task execution
- **Process Management**: Background job control

### Custom Plugin Development
```bash
quantclaw skill create my-plugin
quantclaw skill install ./my-plugin
quantclaw skill status
```

## 🔄 Multi-Provider LLM Support

Support for 7+ language model providers with intelligent failover:

### Supported Providers
- **Anthropic**: Claude models (Haiku, Sonnet, Opus)
- **OpenAI**: GPT-4, GPT-4 Turbo, GPT-3.5
- **Google**: Gemini, PaLM
- **Mistral**: Mistral models
- **Grok**: Xai models
- **OpenRouter**: 100+ models via single API
- **Local**: Ollama, LM Studio (experimental)

### Provider Selection
```json
{
  "agents": {
    "main": {
      "providers": {
        "default": "anthropic",
        "fallback": ["openai", "google", "mistral"]
      },
      "models": {
        "default": "claude-3-5-sonnet-20241022",
        "reasoning": "claude-3-7-opus-20250219"
      }
    }
  }
}
```

### Automatic Failover
- Exponential backoff on failures
- Profile rotation and health monitoring
- Fallback chain execution
- Token usage accumulation

## 🛡️ Enterprise Security

Production-grade security controls:

### Role-Based Access Control (RBAC)
```bash
quantclaw rbac grant user alice operator.read
quantclaw rbac revoke user alice tool.bash.execute
```

### Tool Permissions
- Per-tool allow/deny rules
- Command pattern matching
- Execution approvals
- Audit logging

### Sandboxing
- Process isolation
- File system restrictions
- Network policies
- Resource limits

### Multi-level Authorization
- Agent-level: `agent.admin`, `agent.operator`, `agent.viewer`
- Tool-level: `tool.{name}.read`, `tool.{name}.write`
- Session-level: User-scoped operations

## 📊 Usage Tracking

Comprehensive monitoring and analytics:

```bash
quantclaw usage cost          # Total token costs
quantclaw sessions usage      # Session-by-session breakdown
quantclaw sessions usage.timeseries  # Trends over time
quantclaw logs tail          # Real-time logs
```

### Metrics Collected
- Token usage by provider and model
- Request latency
- Error rates
- Provider health
- Session duration

## 📱 Multi-Channel Integration

Connect across communication platforms:

### Supported Channels
- **Discord**: Rich embeds, reactions, threads
- **Slack**: Blocks, threads, interactive elements
- **Telegram**: Media, inline keyboards
- **WhatsApp**: Text and media messages
- **Email**: HTML and plain text
- **HTTP**: Custom webhooks and APIs
- **Web**: Built-in dashboard

### Channel Configuration
```json
{
  "channels": {
    "discord": {
      "token": "YOUR_BOT_TOKEN",
      "guild_id": "YOUR_GUILD_ID"
    },
    "slack": {
      "token": "xoxb-...",
      "app_id": "..."
    }
  }
}
```

## 🎯 Workflow Automation

Scheduled and triggered automation:

### Cron Jobs
```bash
quantclaw cron schedule "daily-report" "0 9 * * *" "generate-report"
quantclaw cron status
quantclaw cron execute "daily-report"
```

### Hooks and Callbacks
- Pre/post-processing hooks
- Custom event handlers
- Lifecycle callbacks
- Error recovery handlers

## 📊 Performance Optimizations

- **C++17 Native**: Direct compilation for target platform
- **Efficient Memory**: Context compaction and pruning
- **Fast Startup**: Lazy initialization of modules
- **Concurrent Operations**: Multi-threaded request handling
- **Smart Batching**: Grouped API calls

---

**Next**: [Learn about the architecture](/guide/architecture) or [start building plugins](/guide/plugins).
