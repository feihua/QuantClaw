# CLI Reference

Complete command reference for QuantClaw.

## Global Options

```bash
quantclaw [OPTIONS] COMMAND [ARGS]
```

### Options

- `--help, -h` - Show help message
- `--version, -v` - Show version
- `--config PATH` - Configuration file path
- `--log-level LEVEL` - Log level (trace, debug, info, warn, error)
- `--quiet, -q` - Suppress output

## Commands

### agent

Start a QuantClaw agent.

```bash
quantclaw agent [OPTIONS]
```

**Options:**
- `--id ID` - Agent ID (default: main)
- `--port PORT` - Port to listen on (default: 8000)
- `--config PATH` - Config file path
- `--foreground` - Run in foreground
- `--detach, -d` - Run in background

**Examples:**
```bash
# Start main agent
quantclaw agent --id=main

# Start with custom port
quantclaw agent --id=main --port=9000

# Run in background
quantclaw agent --id=main --detach
```

### run

Execute a command through the agent.

```bash
quantclaw run [OPTIONS] MESSAGE
```

**Options:**
- `--session SESSION` - Session ID to use
- `--agent AGENT` - Agent ID to use
- `--no-session` - Run without session (eval mode)
- `--timeout SECONDS` - Request timeout
- `--json` - Output JSON format

**Examples:**
```bash
# Simple question
quantclaw run "What is 2+2?"

# With specific session
quantclaw run --session sess-123 "Continue our conversation"

# Eval without session
quantclaw run --no-session "Generate random number"

# With timeout
quantclaw run --timeout 30 "Analyze this data"

# JSON output
quantclaw run --json "What's the weather?" | jq .result
```

### eval

Evaluate a prompt without session history.

```bash
quantclaw eval [OPTIONS] PROMPT
```

**Options:**
- `--agent AGENT` - Agent ID
- `--timeout SECONDS` - Request timeout
- `--model MODEL` - Use specific model
- `--json` - Output JSON format

**Examples:**
```bash
quantclaw eval "2+2"
quantclaw eval "Generate a poem" --model=reasoning
```

### gateway

Manage the RPC gateway.

```bash
quantclaw gateway COMMAND [OPTIONS]
```

**Commands:**

#### gateway run
Start the gateway server.

```bash
quantclaw gateway run [OPTIONS]
```

**Options:**
- `--host HOST` - Bind address (default: 127.0.0.1)
- `--port PORT` - Port (default: 8000)
- `--tls` - Enable TLS
- `--cert PATH` - Certificate file
- `--key PATH` - Private key file
- `--foreground` - Run in foreground

#### gateway status
Check gateway status.

```bash
quantclaw gateway status
```

**Example Output:**
```
Gateway Status:
  Host: 127.0.0.1:8000
  TLS: Disabled
  Agents: 1
  Uptime: 2h 34m
```

#### gateway stop
Stop the gateway.

```bash
quantclaw gateway stop
```

### sessions

Manage conversation sessions.

```bash
quantclaw sessions COMMAND [OPTIONS]
```

**Commands:**

#### sessions list
List all sessions.

```bash
quantclaw sessions list [OPTIONS]
```

**Options:**
- `--user USER` - Filter by user
- `--agent AGENT` - Filter by agent
- `--limit N` - Limit results
- `--json` - JSON output

**Example:**
```bash
quantclaw sessions list
quantclaw sessions list --user=alice --limit=10
```

#### sessions history
View session history.

```bash
quantclaw sessions history SESSION_ID [OPTIONS]
```

**Options:**
- `--format FORMAT` - Output format (json, text)
- `--limit N` - Limit messages

**Example:**
```bash
quantclaw sessions history sess-123
quantclaw sessions history sess-123 --limit=20
```

#### sessions send
Send message to session.

```bash
quantclaw sessions send SESSION_ID MESSAGE [OPTIONS]
```

**Example:**
```bash
quantclaw sessions send sess-123 "Continue the conversation"
```

#### sessions compact
Compact session history.

```bash
quantclaw sessions compact SESSION_ID [OPTIONS]
```

**Options:**
- `--keep-recent N` - Keep recent N messages

#### sessions delete
Delete a session.

```bash
quantclaw sessions delete SESSION_ID
```

### config

Manage configuration.

```bash
quantclaw config COMMAND [OPTIONS]
```

**Commands:**

#### config get
Get configuration values.

```bash
quantclaw config get [PATH] [OPTIONS]
```

**Options:**
- `--format FORMAT` - Output format (json, yaml)

**Examples:**
```bash
quantclaw config get                                    # Full config
quantclaw config get agents.main.models.default        # Specific value
quantclaw config get agents.main                       # Agent config
```

#### config set
Set configuration value.

```bash
quantclaw config set PATH VALUE
```

**Example:**
```bash
quantclaw config set agents.main.models.default gpt-4
```

#### config validate
Validate configuration.

```bash
quantclaw config validate [PATH]
```

**Example:**
```bash
quantclaw config validate
quantclaw config validate ~/.quantclaw/config.json
```

#### config schema
Show configuration schema.

```bash
quantclaw config schema [PATH]
```

#### config merge
Merge additional configuration.

```bash
quantclaw config merge PATH [OPTIONS]
```

**Options:**
- `--backup` - Create backup before merge

### file

Manage workspace files.

```bash
quantclaw file COMMAND [OPTIONS]
```

**Commands:**

#### file list
List workspace files.

```bash
quantclaw file list [PATH] [OPTIONS]
```

**Options:**
- `--recursive, -r` - Recursive listing
- `--json` - JSON output

**Examples:**
```bash
quantclaw file list
quantclaw file list WORKSPACE/ -r
```

#### file read
Read workspace file.

```bash
quantclaw file read PATH
```

**Example:**
```bash
quantclaw file read WORKSPACE/MEMORY.md
```

#### file write
Write workspace file.

```bash
quantclaw file write PATH CONTENT [OPTIONS]
```

**Options:**
- `--append` - Append to file
- `--overwrite` - Overwrite file

**Example:**
```bash
quantclaw file write WORKSPACE/notes.txt "New note"
quantclaw file write WORKSPACE/data.json '{"key": "value"}'
```

#### file delete
Delete workspace file.

```bash
quantclaw file delete PATH
```

### memory

Manage agent memory.

```bash
quantclaw memory COMMAND [OPTIONS]
```

**Commands:**

#### memory search
Search memory using BM25.

```bash
quantclaw memory search QUERY [OPTIONS]
```

**Options:**
- `--limit N` - Result limit (default: 5)
- `--threshold F` - Score threshold
- `--json` - JSON output

**Example:**
```bash
quantclaw memory search "user preferences"
quantclaw memory search "recent events" --limit=10
```

#### memory get
Get memory by key.

```bash
quantclaw memory get KEY
```

#### memory set
Set memory value.

```bash
quantclaw memory set KEY VALUE
```

#### memory delete
Delete memory entry.

```bash
quantclaw memory delete KEY
```

### skill

Manage skills and plugins.

```bash
quantclaw skill COMMAND [OPTIONS]
```

**Commands:**

#### skill create
Create new skill project.

```bash
quantclaw skill create NAME [OPTIONS]
```

**Options:**
- `--template TEMPLATE` - Use template
- `--typescript` - Use TypeScript

**Example:**
```bash
quantclaw skill create my-plugin
```

#### skill install
Install a plugin.

```bash
quantclaw skill install PATH [OPTIONS]
```

**Options:**
- `--global, -g` - Install globally
- `--link` - Create symlink (for development)

**Example:**
```bash
quantclaw skill install ./my-plugin
quantclaw skill install ./my-plugin --link
```

#### skill uninstall
Uninstall a plugin.

```bash
quantclaw skill uninstall NAME
```

#### skill list
List installed skills.

```bash
quantclaw skill list [OPTIONS]
```

**Options:**
- `--json` - JSON output

#### skill status
Show skill status.

```bash
quantclaw skill status [NAME]
```

#### skill publish
Publish skill to registry.

```bash
quantclaw skill publish [OPTIONS]
```

### tool

Execute tools directly.

```bash
quantclaw tool NAME [PARAMS] [OPTIONS]
```

**Examples:**
```bash
# Bash execution
quantclaw tool bash '{"command": "ls -la"}'

# Browser control
quantclaw tool browser '{"action": "launch", "url": "https://example.com"}'

# Web search
quantclaw tool web_search '{"query": "latest AI news"}'

# Memory search
quantclaw tool memory_search '{"query": "user preferences"}'
```

### onboard

Interactive setup wizard.

```bash
quantclaw onboard [OPTIONS]
```

**Options:**
- `--quick` - Quick setup with defaults
- `--reset` - Reset to defaults
- `--api-key KEY` - Provide API key

**Example:**
```bash
quantclaw onboard                # Interactive
quantclaw onboard --quick        # Quick setup
quantclaw onboard --reset        # Reset config
```

### status

Check system status.

```bash
quantclaw status [OPTIONS]
```

**Options:**
- `--json` - JSON output
- `--verbose, -v` - Verbose output

**Example Output:**
```
QuantClaw Status:
  Version: 1.0.0
  Agent (main): Running
  Gateway: Running on 127.0.0.1:8000
  Memory: 2.4 MB used / 100 MB available
  Sessions: 3 active
  Plugins: 5 installed
```

### logs

View system logs.

```bash
quantclaw logs COMMAND [OPTIONS]
```

**Commands:**

#### logs tail
Show live logs.

```bash
quantclaw logs tail [OPTIONS]
```

**Options:**
- `--lines N` - Number of lines
- `--level LEVEL` - Filter by level
- `--component COMP` - Filter by component
- `--follow, -f` - Follow logs

**Examples:**
```bash
quantclaw logs tail
quantclaw logs tail --lines=100 --level=error
quantclaw logs tail -f --component=agent
```

#### logs search
Search logs.

```bash
quantclaw logs search PATTERN [OPTIONS]
```

**Options:**
- `--limit N` - Result limit

### usage

View usage statistics.

```bash
quantclaw usage COMMAND [OPTIONS]
```

**Commands:**

#### usage cost
Show token costs.

```bash
quantclaw usage cost [OPTIONS]
```

**Options:**
- `--period PERIOD` - Period (day, week, month)
- `--by FIELD` - Group by (provider, model, agent)

**Example:**
```bash
quantclaw usage cost
quantclaw usage cost --period=month --by=provider
```

#### usage stats
Show usage statistics.

```bash
quantclaw usage stats [OPTIONS]
```

### help

Show help.

```bash
quantclaw help [COMMAND]
```

**Examples:**
```bash
quantclaw help                # Main help
quantclaw help agent          # Agent command help
quantclaw help run            # Run command help
```

## Exit Codes

- `0` - Success
- `1` - General error
- `2` - Command line error
- `3` - Configuration error
- `4` - Connection error
- `5` - Permission error

## Environment Variables

- `QUANTCLAW_CONFIG_DIR` - Configuration directory
- `QUANTCLAW_AGENT_ID` - Default agent ID
- `QUANTCLAW_LOG_LEVEL` - Log level
- `QUANTCLAW_GATEWAY_PORT` - Gateway port
- `QUANTCLAW_PORT` - Sidecar IPC port (internal)

## Examples

### Complete Workflow

```bash
# 1. Initial setup
quantclaw onboard --quick

# 2. Start gateway
quantclaw gateway run &

# 3. Start agent
quantclaw agent --id=main &

# 4. Send message
quantclaw run "What is 2+2?"

# 5. View session history
quantclaw sessions list
quantclaw sessions history SESS-ID

# 6. Check usage
quantclaw usage cost

# 7. View logs
quantclaw logs tail -f
```

---

**Need help with a specific command?** Use `quantclaw help COMMAND`.
