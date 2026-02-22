#include "quantclaw/gateway/gateway_server.hpp"
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace quantclaw::gateway {

GatewayServer::GatewayServer(int port, std::shared_ptr<spdlog::logger> logger)
    : port_(port), logger_(logger) {
    logger_->info("GatewayServer created on port {}", port_);
}

GatewayServer::~GatewayServer() {
    stop();
}

void GatewayServer::start() {
    if (running_) {
        logger_->warn("GatewayServer already running");
        return;
    }

    auto ws_server = std::make_unique<HttpAwareWebSocketServer>(port_, "0.0.0.0");
    ws_server->setHttpRedirectPort(http_redirect_port_);
    server_ = std::move(ws_server);

    server_->setOnClientMessageCallback(
        [this](std::shared_ptr<ix::ConnectionState> state,
               ix::WebSocket& ws,
               const ix::WebSocketMessagePtr& msg) {
            on_connection(state, ws, msg);
        }
    );

    auto res = server_->listen();
    if (!res.first) {
        throw std::runtime_error("Failed to listen on port " + std::to_string(port_) +
                                 ": " + res.second);
    }

    server_->start();
    running_ = true;
    start_time_ = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    logger_->info("GatewayServer started on port {}", port_);
}

void GatewayServer::stop() {
    if (!running_) return;

    running_ = false;
    if (server_) {
        server_->stop();
        server_.reset();
    }

    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.clear();
    ws_connections_.clear();

    logger_->info("GatewayServer stopped");
}

bool GatewayServer::is_running() const {
    return running_;
}

void GatewayServer::register_handler(const std::string& method, RpcHandler handler) {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    handlers_[method] = std::move(handler);
    logger_->debug("Registered RPC handler: {}", method);
}

void GatewayServer::broadcast_event(const std::string& event, const nlohmann::json& payload) {
    RpcEvent evt;
    evt.event = event;
    evt.payload = payload;
    std::string msg = evt.to_json().dump();

    std::lock_guard<std::mutex> lock(connections_mutex_);
    for (auto& [id, ws] : ws_connections_) {
        if (ws) {
            ws->send(msg);
        }
    }
}

void GatewayServer::send_event_to(const std::string& connection_id, const RpcEvent& event) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = ws_connections_.find(connection_id);
    if (it != ws_connections_.end() && it->second) {
        it->second->send(event.to_json().dump());
    }
}

size_t GatewayServer::get_connection_count() const {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    return connections_.size();
}

int64_t GatewayServer::get_uptime_seconds() const {
    if (!running_ || start_time_ == 0) return 0;
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    return now - start_time_;
}

void GatewayServer::on_connection(std::shared_ptr<ix::ConnectionState> state,
                                   ix::WebSocket& ws,
                                   const ix::WebSocketMessagePtr& msg) {
    std::string conn_id = state->getId();

    switch (msg->type) {
        case ix::WebSocketMessageType::Open: {
            logger_->info("Client connected: {}", conn_id);

            {
                std::lock_guard<std::mutex> lock(connections_mutex_);
                ClientConnection client;
                client.connection_id = conn_id;
                client.connected_at = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()
                ).count();
                connections_[conn_id] = client;
                ws_connections_[conn_id] = &ws;
            }

            // Send challenge
            send_challenge(ws);
            break;
        }

        case ix::WebSocketMessageType::Close: {
            logger_->info("Client disconnected: {}", conn_id);
            std::lock_guard<std::mutex> lock(connections_mutex_);
            connections_.erase(conn_id);
            ws_connections_.erase(conn_id);
            break;
        }

        case ix::WebSocketMessageType::Message: {
            handle_message(conn_id, ws, msg->str);
            break;
        }

        case ix::WebSocketMessageType::Error: {
            logger_->error("WebSocket error for {}: {}", conn_id, msg->errorInfo.reason);
            break;
        }

        default:
            break;
    }
}

void GatewayServer::handle_message(const std::string& conn_id,
                                    ix::WebSocket& ws,
                                    const std::string& data) {
    try {
        auto j = nlohmann::json::parse(data);
        auto type = parse_frame_type(j);

        switch (type) {
            case FrameType::Request: {
                auto request = RpcRequest::from_json(j);

                // Special handling for connect.hello / connect (OpenClaw)
                bool is_openclaw = (request.method == methods::OC_CONNECT);
                if (request.method == methods::CONNECT_HELLO || is_openclaw) {
                    bool ok = handle_hello(conn_id, request.params, is_openclaw);
                    if (ok) {
                        HelloOkPayload hello_ok;
                        hello_ok.openclaw_format = is_openclaw;
                        auto resp = RpcResponse::success(request.id, hello_ok.to_json());
                        ws.send(resp.to_json().dump());
                    } else {
                        auto resp = RpcResponse::failure(request.id, "Authentication failed");
                        ws.send(resp.to_json().dump());
                    }
                    return;
                }

                handle_rpc_request(conn_id, ws, request);
                break;
            }

            default:
                logger_->warn("Unexpected frame type from client: {}", data);
                break;
        }
    } catch (const std::exception& e) {
        logger_->error("Failed to parse message from {}: {}", conn_id, e.what());
    }
}

void GatewayServer::handle_rpc_request(const std::string& conn_id,
                                        ix::WebSocket& ws,
                                        const RpcRequest& request) {
    logger_->info("RPC request from {}: {} (id={})", conn_id, request.method, request.id);

    RpcHandler handler;
    {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        auto it = handlers_.find(request.method);
        if (it == handlers_.end()) {
            auto resp = RpcResponse::failure(request.id,
                                              "Unknown method: " + request.method);
            ws.send(resp.to_json().dump());
            return;
        }
        handler = it->second;
    }

    ClientConnection* client = nullptr;
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        auto it = connections_.find(conn_id);
        if (it != connections_.end()) {
            client = &it->second;
        }
    }

    if (!client) {
        auto resp = RpcResponse::failure(request.id, "Connection not found");
        ws.send(resp.to_json().dump());
        return;
    }

    // Enforce authentication: if auth mode is not "none", client must have sent hello
    if (auth_mode_ != "none" && !client->authenticated) {
        auto resp = RpcResponse::failure(request.id,
                                          "Not authenticated: send connect.hello first");
        ws.send(resp.to_json().dump());
        return;
    }

    try {
        auto result = handler(request.params, *client);
        auto resp = RpcResponse::success(request.id, result);
        ws.send(resp.to_json().dump());
    } catch (const std::exception& e) {
        logger_->error("RPC handler error for {}: {}", request.method, e.what());
        auto resp = RpcResponse::failure(request.id, e.what());
        ws.send(resp.to_json().dump());
    }
}

void GatewayServer::send_challenge(ix::WebSocket& ws) {
    // Generate nonce
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex = "0123456789abcdef";

    std::string nonce;
    nonce.reserve(32);
    for (int i = 0; i < 32; ++i) {
        nonce += hex[dis(gen)];
    }

    ConnectChallenge challenge;
    challenge.nonce = nonce;
    challenge.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    ws.send(challenge.to_json().dump());
}

void GatewayServer::set_auth(const std::string& mode, const std::string& token) {
    auth_mode_ = mode;
    expected_token_ = token;
    logger_->info("Gateway auth configured: mode={}", mode);
}

bool GatewayServer::handle_hello(const std::string& conn_id,
                                  const nlohmann::json& params,
                                  bool is_openclaw) {
    auto hello = ConnectHelloParams::from_json(params);

    // If auth mode is "token", validate the token
    if (auth_mode_ == "token" && !expected_token_.empty()) {
        if (hello.auth_token != expected_token_) {
            logger_->warn("Auth failed for {}: bad token", conn_id);
            return false;
        }
    }
    // auth mode "none" → skip validation

    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = connections_.find(conn_id);
    if (it == connections_.end()) {
        return false;
    }

    it->second.role = hello.role;
    it->second.scopes = hello.scopes;
    it->second.device_id = hello.device_id;
    it->second.client_name = hello.client_name;
    it->second.client_version = hello.client_version;
    it->second.authenticated = true;
    it->second.client_type = is_openclaw ? "openclaw" : "quantclaw";

    logger_->info("Client {} authenticated: role={}, client={}, type={}",
                  conn_id, hello.role, hello.client_name, it->second.client_type);
    return true;
}

} // namespace quantclaw::gateway
