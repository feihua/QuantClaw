# QuantClaw Project Status

## Overview

**QuantClaw** is a C++17 implementation of OpenClaw, a personal AI assistant.

**Current status**: Core features implemented. Gateway (WebSocket + HTTP API), session persistence, multi-provider LLM, tool system, channel adapters, and CLI are all functional with 324 passing tests.

## Implemented

### Core
- **AgentLoop**: Message processing loop with multi-turn tool calling (streaming + non-streaming), returns full message history for persistence
- **MemoryManager**: Workspace file loading, identity files (SOUL.md, USER.md), daily memory, file watcher
- **SkillLoader**: SKILL.md parsing, environment gating, multi-directory loading
- **PromptBuilder**: System prompt construction from SOUL + skills + tools + runtime info
- **Config**: YAML/JSON configuration with hot-reload via SIGHUP
- **SignalHandler**: Graceful shutdown (SIGTERM) and config reload (SIGHUP)

### LLM Integration
- **OpenAI Provider**: Chat completion + streaming via libcurl
- **Anthropic Provider**: Anthropic API with Messages format + streaming
- **Model routing**: `provider/model-name` prefix routing (e.g. `openai/qwen-max`)

### Gateway
- **GatewayServer**: WebSocket RPC server (IXWebSocket) with JSON-RPC protocol
  - Nonce-challenge authentication with Bearer token
  - 11 RPC methods: health, status, config, agent request/stop, sessions CRUD, channels, chain execute
  - Real-time streaming events (text_delta, tool_use, tool_result, message_end)
- **GatewayClient**: WebSocket RPC client for CLI commands
- **DaemonManager**: systemd/launchd service management (install, start, stop, restart)

### HTTP API (Control Panel)
- **WebServer**: HTTP REST server (cpp-httplib) with CORS and Bearer token auth
- **10 REST endpoints** mirroring WebSocket RPC:
  - `GET /api/health` — status, uptime, version
  - `GET /api/status` — gateway state with connections/sessions count
  - `GET /api/config` — configuration values (optional `?path=` dot-path)
  - `POST /api/agent/request` — non-streaming agent request
  - `POST /api/agent/stop` — stop agent processing
  - `GET /api/sessions` — list sessions (`?limit=&offset=`)
  - `GET /api/sessions/history` — session history (`?sessionKey=`)
  - `POST /api/sessions/delete` — delete session
  - `POST /api/sessions/reset` — reset session
  - `GET /api/channels` — list channels
- Default port: 18790 (configurable via `gateway.controlUi.port`)

### Channel Adapters
- **ChannelAdapterManager**: Manages external channel adapters (Node.js child processes)
- **Discord Adapter**: Discord bot integration via discord.js — receives messages, forwards to gateway via WebSocket RPC, streams replies back
- **Telegram Adapter**: Telegram bot integration via telegraf — same architecture as Discord
- Adapters connect to the gateway as standard WebSocket RPC clients (`connect` + `chat.send`)

### Session Persistence
- **SessionManager**: Full conversation persistence in JSONL format
  - ContentBlock-level storage (text, tool_use, tool_result preserved)
  - Session create/delete/reset/list/history
  - Auto-generated display names from first user message
  - `created_at` / `updated_at` timestamps
  - sessions.json index + per-session JSONL transcript files

### Tools
- **ToolRegistry**: File read/write/edit, shell exec, message sending
- **ToolChain**: Multi-step tool execution pipelines
- **ToolPermissions**: Group-based allow/deny rules
- **Security Sandbox**: Path validation, command validation, resource limits

### MCP (Model Context Protocol)
- **MCPServer**: JSON-RPC server with tool registration
- **MCPClient**: JSON-RPC client for remote tool calls
- **MCPToolManager**: Auto-discovery and registration of MCP tools into ToolRegistry

### CLI
- **CLIManager**: Command routing with aliases and help display
- **Commands**: `gateway` (foreground/install/start/stop/restart/status), `agent`, `sessions` (list/history/delete/reset), `status`, `health`, `config`, `skills`, `doctor`

## Tech Stack

- **Language**: C++17
- **Build System**: CMake 3.20+
- **Logging**: spdlog
- **JSON**: nlohmann/json
- **HTTP Client**: libcurl
- **HTTP Server**: cpp-httplib 0.18.3
- **WebSocket**: IXWebSocket 11.4.5
- **TLS**: OpenSSL
- **Testing**: Google Test 1.14.0 (324 tests)

## Project Rules

- **No Boost**: This project does not use Boost
- Lightweight dependencies only

## Not Yet Implemented

- TUI interactive mode
- Multiple agent profiles

## Last Updated

2026-02-24 | Version: 0.2.0
