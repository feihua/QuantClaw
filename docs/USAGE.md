# QuantClaw 使用指南

## 快速开始

### 1. 安装

#### 从源码安装
```bash
git clone https://github.com/openclaw/quantclaw.git
cd quantclaw
./scripts/install.sh
```

#### Docker 安装
```bash
docker-compose up -d
```

### 2. 配置

编辑配置文件 `~/.quantclaw/config.json`:

```json
{
  "agents": {
    "defaults": {
      "model": "gpt-4-turbo",
      "max_iterations": 15,
      "temperature": 0.7
    }
  },
  "providers": {
    "openai": {
      "api_key": "sk-your-api-key",
      "base_url": "https://api.openai.com/v1"
    }
  },
  "channels": {
    "discord": {
      "enabled": true,
      "token": "your-discord-bot-token"
    }
  }
}
```

### 3. 启动

#### 作为服务启动
```bash
systemctl start quantclaw
systemctl enable quantclaw  # 开机自启
```

#### 直接运行
```bash
quantclaw gateway start
```

#### 前台模式（调试用）
```bash
quantclaw gateway start --foreground
```

## CLI 命令

### Gateway 管理
```bash
# 查看状态
quantclaw gateway status

# 启动
quantclaw gateway start

# 停止
quantclaw gateway stop

# 重启
quantclaw gateway restart
```

### Agent 功能
```bash
# 发送消息
quantclaw agent --message "Hello, introduce yourself"

# 指定对话 ID
quantclaw agent --message "What's the weather?" --conversation-id weather-chat
```

### 技能管理
```bash
# 列出技能
quantclaw skills list

# 安装技能
quantclaw skills install weather
```

### 配置管理
```bash
# 显示配置
quantclaw config show

# 设置配置项
quantclaw config set agents.defaults.model gpt-4-turbo
```

## Web API

### REST API

#### 发送消息
```bash
curl -X POST http://localhost:8080/api/v1/agent \
  -H "Content-Type: application/json" \
  -d '{
    "message": "Hello!",
    "conversation_id": "my-chat"
  }'
```

#### 查看 Gateway 状态
```bash
curl http://localhost:8080/api/v1/gateway/status
```

#### 列出技能
```bash
curl http://localhost:8080/api/v1/skills
```

#### 健康检查
```bash
curl http://localhost:8080/api/v1/health
```

### WebSocket API

```javascript
const ws = new WebSocket('ws://localhost:8080/ws');

ws.onopen = () => {
  ws.send(JSON.stringify({
    message: "Hello!",
    conversation_id: "my-chat"
  }));
};

ws.onmessage = (event) => {
  const response = JSON.parse(event.data);
  console.log(response.content);
};
```

## Discord 集成

### 1. 创建 Discord Bot

1. 访问 https://discord.com/developers/applications
2. 创建新应用
3. 创建 Bot 并复制 Token
4. 邀请 Bot 到你的服务器

### 2. 配置 QuantClaw

在 `config.json` 中添加:

```json
{
  "channels": {
    "discord": {
      "enabled": true,
      "token": "your-bot-token-here",
      "allowed_ids": ["your-user-id"]
    }
  }
}
```

### 3. 重启服务

```bash
systemctl restart quantclaw
```

## 故障排除

### 查看日志
```bash
# systemd 日志
journalctl -u quantclaw -f

# 文件日志
tail -f /var/log/quantclaw/quantclaw.log
```

### 常见问题

**Q: Gateway 无法启动**
```bash
# 检查配置文件
quantclaw config show

# 检查端口占用
netstat -tlnp | grep 8080

# 查看详细错误
journalctl -u quantclaw -n 50
```

**Q: Discord 连接失败**
- 检查 Bot Token 是否正确
- 检查 Bot 是否已邀请到服务器
- 检查网络连通性

**Q: API 调用失败**
- 检查 LLM API Key 是否有效
- 检查网络连接
- 查看日志中的详细错误信息

## 性能调优

### 内存限制
在 systemd 服务文件中调整:
```ini
MemoryLimit=512M
CPUQuota=200%
```

### 并发连接
调整 Web 服务器配置:
```json
{
  "web": {
    "max_connections": 100,
    "max_requests_per_second": 50
  }
}
```

## 安全最佳实践

1. **不要以 root 用户运行**
2. **使用防火墙限制访问**
3. **定期更新依赖**
4. **监控资源使用**
5. **启用日志审计**

## 更多信息

- GitHub: https://github.com/openclaw/quantclaw
- 文档: https://docs.quantclaw.ai
- 社区: https://discord.gg/quantclaw