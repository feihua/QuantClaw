# Dashboard Analysis: QuantClaw vs OpenClaw

## 1. QuantClaw Current State

QuantClaw's "Control UI" is a **pure JSON REST API server** with **zero frontend**.

- HTTP server using `cpp-httplib`, default port `18790`
- 11 REST endpoints: health, status, config, agent request/stop, sessions CRUD, channels, config reload
- CORS + Bearer token auth
- **No HTML, CSS, JS, or any static file serving**

### Existing API Endpoints

| Method | Path | Purpose |
|--------|------|---------|
| GET | `/api/health` | Health check |
| GET | `/api/status` | Gateway status + metrics |
| GET | `/api/config` | Read config (optional `?path=`) |
| POST | `/api/config/reload` | Hot-reload config |
| POST | `/api/agent/request` | Send message to agent |
| POST | `/api/agent/stop` | Stop agent processing |
| GET | `/api/sessions` | List sessions |
| GET | `/api/sessions/history` | Session history |
| POST | `/api/sessions/delete` | Delete session |
| POST | `/api/sessions/reset` | Reset session |
| GET | `/api/channels` | List channels |

---

## 2. OpenClaw Dashboard

OpenClaw ships a **complete web dashboard** (Control UI) built into its Gateway.

### Tech Stack

| Component | Technology |
|-----------|------------|
| Framework | **Lit 3.x** (Web Components) |
| Build tool | **Vite 7.x** |
| Language | **TypeScript** |
| Testing | Vitest + Playwright |
| Package manager | pnpm |

### Features

| Feature | QuantClaw | OpenClaw |
|---------|-----------|----------|
| Chat interface (streaming) | API only | Full UI with tool calls, abort, assistant notes |
| Session management | API only | Visual list with model badges, context %, token counts |
| Channel management | API only | WhatsApp/Telegram/Discord/Slack/Signal monitoring |
| Config editing | API only | Form rendering + raw JSON editor |
| Cost tracking | None | Per-model token/cost breakdown (7d/30d/all-time) |
| i18n | None | Yes |
| Device pairing | None | Yes |
| Exec approvals | None | Yes |
| Cron management | None | Yes |

### License

**MIT License** (Copyright Peter Steinberger 2025). Fully compatible with QuantClaw. Can freely use, modify, and distribute with copyright notice retained.

Source: https://github.com/openclaw/openclaw/blob/main/LICENSE

---

## 3. Protocol Compatibility Analysis

OpenClaw's UI connects to its Gateway via **WebSocket**. QuantClaw also uses WebSocket RPC. Both use JSON frames with `{type, id, method, params}` structure. However, there are **critical incompatibilities**.

### 3.1 Frame Format

| Aspect | QuantClaw | OpenClaw | Compatible |
|--------|-----------|----------|------------|
| Request frame | `{type:"req", id, method, params}` | Same | YES |
| Response (success) | `{type:"res", id, ok:true, payload}` | Same | YES |
| Response (error) | `error: "string"` | `error: {code, message, details?, retryable?}` | **NO** |
| Event frame | `{type:"event", event, payload}` | Same | YES |
| `stateVersion` | `uint64` scalar | `{presence: int, health: int}` | **NO** |

### 3.2 Authentication Handshake

| Aspect | QuantClaw | OpenClaw | Compatible |
|--------|-----------|----------|------------|
| Method name | `"connect.hello"` | `"connect"` | **NO** |
| Protocol version | 1 | 3 | **NO** |
| Client identity | flat `clientName`, `clientVersion` | nested `client: {id, version, platform}` | **NO** |
| Auth field | flat `authToken` | nested `auth: {token, password?, deviceToken?}` | **NO** |
| Device field | flat `deviceId` | nested `device: {id, publicKey, signature}` | **NO** |
| Challenge timestamp | seconds | milliseconds | **NO** |
| Hello response | `{protocol, policy:"permissive", authenticated, tickIntervalMs}` | `{type:"hello-ok", protocol, server, features, snapshot, auth, policy:{...}}` | **NO** |

### 3.3 RPC Method Names

| Function | QuantClaw | OpenClaw | Match |
|----------|-----------|----------|-------|
| Health | `gateway.health` | `health` | NO |
| Status | `gateway.status` | `status` | NO |
| Config read | `config.get` | `config.get` | YES |
| Config reload | `config.reload` | N/A | -- |
| Chat send | `agent.request` | `chat.send` | NO |
| Chat stop | `agent.stop` | `chat.abort` | NO |
| Session list | `sessions.list` | `sessions.list` | YES |
| Session history | `sessions.history` | `chat.history` | NO |
| Session delete | `sessions.delete` | `sessions.delete` | YES |
| Session reset | `sessions.reset` | `sessions.reset` | YES |
| Channel list | `channels.list` | `channels.status` | NO |
| Keepalive event | `gateway.tick` | `tick` | NO |

OpenClaw has ~80+ methods total; QuantClaw has 12.

### 3.4 Event Streaming

| Aspect | QuantClaw | OpenClaw | Compatible |
|--------|-----------|----------|------------|
| Text streaming | `"agent.text_delta"` events | `"chat"` event with `state:"delta"` | **NO** |
| Tool use | `"agent.tool_use"` event | `"agent"` event with `stream:"tool"` | **NO** |
| Completion | `"agent.message_end"` event | `"chat"` event with `state:"final"` | **NO** |
| Run tracking | No `runId` concept | `runId` on every event | **MISSING** |
| Abort event | None | `state:"aborted"` | **MISSING** |

### 3.5 Session Parameter Naming

| Method | QuantClaw param | OpenClaw param |
|--------|----------------|----------------|
| sessions.delete | `sessionKey` | `key` |
| sessions.reset | `sessionKey` | `key` |

---

## 4. Integration Risk Assessment

### License Risk: **NONE**

MIT license is maximally permissive. No copyleft, no patent concerns.

### Protocol Risk: **HIGH**

The protocols are fundamentally incompatible. Dropping OpenClaw's UI onto QuantClaw's Gateway will fail immediately at the WebSocket handshake (`connect` vs `connect.hello`), and even if bypassed, event streaming, method names, and response shapes all differ.

### Effort to Adapt

| Priority | Items | Effort |
|----------|-------|--------|
| Critical (handshake) | Rename `connect.hello` -> `connect`, restructure params, bump protocol to 3, restructure hello-ok response | Medium |
| Critical (streaming) | Replace per-event-type model with `chat`/`agent` event model, add `runId` tracking | Medium-High |
| Critical (errors) | Change error from string to `{code, message}` object | Low |
| High (method names) | Rename 7 methods to match OpenClaw conventions | Low |
| High (missing methods) | Implement `config.set/apply/patch/schema`, `models.list`, `tools.catalog`, `sessions.preview/patch` | High |
| Medium (features) | Device pairing, exec approvals, cron, cost tracking backend | High |

---

## 5. Recommendation

**Do not drop in OpenClaw's UI as-is.** The protocol gap requires a compatibility layer.

### Recommended Approach: Protocol Adapter (Shim)

Instead of rewriting QuantClaw's entire protocol stack, add a **thin WebSocket proxy/shim** that:

1. Accepts OpenClaw-protocol connections from the UI
2. Translates method names, parameter shapes, and events to/from QuantClaw's protocol
3. Runs in-process alongside the existing Gateway

This isolates protocol translation from core logic and allows incremental convergence toward OpenClaw's protocol over time.

### Implementation Phases

**Phase 1 - Static UI Serving + Shim Bootstrap**
- Build OpenClaw's `ui/` to static `dist/`
- Serve from QuantClaw's WebServer at `/`
- Add WebSocket shim endpoint for `connect` handshake

**Phase 2 - Core RPC Translation**
- Map `chat.send` <-> `agent.request`
- Map `chat.abort` <-> `agent.stop`
- Map `chat.history` <-> `sessions.history`
- Translate streaming events between formats

**Phase 3 - Feature Parity**
- Implement missing methods the UI calls (`config.schema`, `models.list`, etc.)
- Add stub responses for features not yet supported (cron, device pairing, etc.)
