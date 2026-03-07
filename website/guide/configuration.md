# Configuration Guide

Configure QuantClaw for your specific use case.

## Configuration File Locations

QuantClaw looks for configuration in this order:

1. **Environment Variable**: `$QUANTCLAW_CONFIG_DIR/config.json`
2. **Default Location**: `~/.quantclaw/config.json`
3. **Custom Path**: Passed via `--config` flag

## Configuration Structure

```json
{
  "version": "1.0",
  "agents": {
    "main": {
      "providers": {
        "default": "anthropic",
        "fallback": ["openai", "google"],
        "timeout_ms": 60000
      },
      "models": {
        "default": "claude-3-5-sonnet-20241022",
        "reasoning": "claude-3-7-opus-20250219",
        "fast": "claude-3-haiku-20240307"
      },
      "memory": {
        "maxTokens": 100000,
        "compactionThreshold": 80000,
        "enableSearch": true
      },
      "context": {
        "maxContextTokens": 32000,
        "windowSize": 10
      }
    }
  },
  "gateway": {
    "host": "127.0.0.1",
    "port": 8000,
    "enableTls": false,
    "certPath": null,
    "keyPath": null
  },
  "logging": {
    "level": "info",
    "format": "json",
    "file": "~/.quantclaw/quantclaw.log"
  },
  "plugins": {
    "sidecarPort": 0,
    "autoLoad": true,
    "pluginDirs": ["~/.quantclaw/plugins"]
  }
}
```

## Agent Configuration

### Provider Configuration

```json
{
  "agents": {
    "main": {
      "providers": {
        "default": "anthropic",
        "fallback": ["openai", "google"],
        "timeout_ms": 60000
      }
    }
  }
}
```

**Options:**
- `default`: Primary provider to use
- `fallback`: Providers to try if default fails
- `timeout_ms`: Request timeout in milliseconds

### Model Configuration

```json
{
  "agents": {
    "main": {
      "models": {
        "default": "claude-3-5-sonnet-20241022",
        "reasoning": "claude-3-7-opus-20250219",
        "fast": "claude-3-haiku-20240307"
      }
    }
  }
}
```

**Predefined Profiles:**

**Anthropic**
- `claude-3-7-opus-20250219` - Most capable, slowest
- `claude-3-5-sonnet-20241022` - Best balanced
- `claude-3-haiku-20240307` - Fast, less capable

**OpenAI**
- `gpt-4-turbo` - Most capable
- `gpt-4o` - Fast and capable
- `gpt-3.5-turbo` - Fast, basic tasks

**Google**
- `gemini-2.0-flash` - High performance
- `gemini-pro` - Balanced

### Memory Configuration

```json
{
  "agents": {
    "main": {
      "memory": {
        "maxTokens": 100000,
        "compactionThreshold": 80000,
        "enableSearch": true,
        "searchTopK": 5
      }
    }
  }
}
```

**Options:**
- `maxTokens`: Maximum memory size in tokens
- `compactionThreshold`: Trigger compaction at % full
- `enableSearch`: Enable BM25 memory search
- `searchTopK`: Number of results to return

### Context Configuration

```json
{
  "agents": {
    "main": {
      "context": {
        "maxContextTokens": 32000,
        "windowSize": 10,
        "overflowStrategy": "compaction"
      }
    }
  }
}
```

**Options:**
- `maxContextTokens`: Maximum context size
- `windowSize`: Recent messages to keep
- `overflowStrategy`: "compaction", "truncate", or "error"

### Tool Configuration

```json
{
  "agents": {
    "main": {
      "tools": {
        "bash": {
          "enabled": true,
          "timeout_ms": 30000,
          "maxOutputSize": 10000,
          "allowedCommands": ["ls", "pwd", "grep"]
        },
        "browser": {
          "enabled": true,
          "headless": true,
          "timeout_ms": 60000
        }
      }
    }
  }
}
```

### Environment Variables

```json
{
  "agents": {
    "main": {
      "env": {
        "OPENAI_API_KEY": "${OPENAI_API_KEY}",
        "GITHUB_TOKEN": "${GITHUB_TOKEN}"
      }
    }
  }
}
```

Variables are automatically replaced from system environment.

## Gateway Configuration

```json
{
  "gateway": {
    "host": "0.0.0.0",
    "port": 8000,
    "enableTls": true,
    "certPath": "/path/to/cert.pem",
    "keyPath": "/path/to/key.pem",
    "cors": {
      "enabled": true,
      "allowedOrigins": ["https://example.com"]
    }
  }
}
```

## Logging Configuration

```json
{
  "logging": {
    "level": "info",
    "format": "json",
    "file": "~/.quantclaw/quantclaw.log",
    "maxFileSize": 10485760,
    "maxFiles": 5,
    "console": true
  }
}
```

**Log Levels:**
- `trace`: Most verbose
- `debug`: Debugging information
- `info`: General information
- `warn`: Warnings
- `error`: Errors only
- `critical`: Critical errors only

## Plugin Configuration

```json
{
  "plugins": {
    "sidecarPort": 0,
    "autoLoad": true,
    "pluginDirs": [
      "~/.quantclaw/plugins",
      "/usr/share/quantclaw/plugins"
    ],
    "disabled": ["old-plugin"]
  }
}
```

**Options:**
- `sidecarPort`: Port for Node.js sidecar (0 = auto-select)
- `autoLoad`: Auto-load plugins on startup
- `pluginDirs`: Directories to scan for plugins
- `disabled`: Plugins to skip loading

## Channel Configuration

```json
{
  "channels": {
    "discord": {
      "enabled": true,
      "token": "${DISCORD_BOT_TOKEN}",
      "guildId": "123456789"
    },
    "slack": {
      "enabled": true,
      "token": "${SLACK_BOT_TOKEN}",
      "appId": "A123456789"
    },
    "telegram": {
      "enabled": false,
      "token": "${TELEGRAM_BOT_TOKEN}"
    }
  }
}
```

## Configuration Commands

### View Configuration

```bash
# Show full configuration
quantclaw config get

# Get specific value
quantclaw config get agents.main.models.default

# Get agent config
quantclaw config get agents.main
```

### Validate Configuration

```bash
# Validate syntax and structure
quantclaw config validate

# Show configuration schema
quantclaw config schema
```

### Set Configuration

```bash
# Set value
quantclaw config set agents.main.models.default gpt-4

# Merge configuration
quantclaw config merge path/to/additional.json
```

## Environment Variable Substitution

Configuration supports `${VAR}` substitution:

```json
{
  "agents": {
    "main": {
      "env": {
        "API_KEY": "${API_KEY}",
        "HOME": "${HOME}"
      }
    }
  }
}
```

Set environment before running:
```bash
export API_KEY=your-secret-key
quantclaw agent --id=main
```

## Configuration Examples

### Minimal Setup

```json
{
  "agents": {
    "main": {
      "providers": {
        "default": "anthropic"
      },
      "models": {
        "default": "claude-3-5-sonnet-20241022"
      }
    }
  }
}
```

### Multi-Provider Setup

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
        "reasoning": "claude-3-7-opus-20250219",
        "fast": "gpt-3.5-turbo"
      }
    }
  }
}
```

### High-Memory Setup

```json
{
  "agents": {
    "main": {
      "memory": {
        "maxTokens": 200000,
        "compactionThreshold": 160000,
        "enableSearch": true,
        "searchTopK": 10
      },
      "context": {
        "maxContextTokens": 64000,
        "windowSize": 20
      }
    }
  }
}
```

### Production Setup

```json
{
  "gateway": {
    "host": "0.0.0.0",
    "port": 8000,
    "enableTls": true,
    "certPath": "/etc/certs/cert.pem",
    "keyPath": "/etc/certs/key.pem"
  },
  "logging": {
    "level": "warn",
    "format": "json",
    "file": "/var/log/quantclaw/quantclaw.log"
  },
  "plugins": {
    "autoLoad": true,
    "pluginDirs": ["/usr/share/quantclaw/plugins"]
  }
}
```

## Tips and Best Practices

- **Use environment variables** for secrets (API keys, tokens)
- **Start with minimal config**, add features as needed
- **Validate after changes**: `quantclaw config validate`
- **Monitor logs** during initial setup
- **Use different agents** for different use cases
- **Test model fallbacks** before production
- **Set reasonable timeouts** to prevent hanging

---

**Next**: [View CLI reference](/guide/cli-reference) or [get started](/guide/getting-started).
