#pragma once

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXHttp.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "quantclaw/gateway/protocol.hpp"

namespace quantclaw::gateway {

// WebSocket server that handles plain HTTP requests gracefully
// instead of logging "Missing Sec-WebSocket-Key" errors.
// When a browser or HTTP client hits the WS port, it returns a redirect
// to the Control UI HTTP port.
class HttpAwareWebSocketServer : public ix::WebSocketServer {
public:
    using ix::WebSocketServer::WebSocketServer;

    void setHttpRedirectPort(int port) { http_port_ = port; }

private:
    int http_port_ = 0;

    void handleConnection(std::unique_ptr<ix::Socket> socket,
                          std::shared_ptr<ix::ConnectionState> connectionState) override
    {
        // Parse the incoming HTTP request from the raw socket
        auto [ok, errMsg, request] = ix::Http::parseRequest(socket, getHandshakeTimeoutSecs());
        if (!ok) {
            // Malformed request — just close
            connectionState->setTerminated();
            return;
        }

        // Check if this is a proper WebSocket upgrade request
        bool has_ws_key = request->headers.find("sec-websocket-key") != request->headers.end();
        if (has_ws_key) {
            // Real WebSocket client — proceed with the normal upgrade path,
            // passing the already-parsed request so bytes aren't re-read.
            handleUpgrade(std::move(socket), connectionState, request);
            connectionState->setTerminated();
            return;
        }

        // Plain HTTP request (browser, curl, health-check, etc.)
        auto resp = std::make_shared<ix::HttpResponse>();
        if (http_port_ > 0) {
            resp->statusCode = 301;
            resp->description = "Moved Permanently";
            resp->headers["Location"] = "http://localhost:" + std::to_string(http_port_) + "/";
            resp->headers["Content-Type"] = "text/html";
            resp->body = "<html><body>Redirecting to <a href=\"http://localhost:"
                + std::to_string(http_port_) + "/\">dashboard</a></body></html>\n";
        } else {
            resp->statusCode = 426;
            resp->description = "Upgrade Required";
            resp->headers["Upgrade"] = "websocket";
            resp->headers["Content-Type"] = "text/plain";
            resp->body = "This is a WebSocket endpoint. Use a WebSocket client to connect.\n";
        }
        ix::Http::sendResponse(resp, socket);
        connectionState->setTerminated();
    }
};

using RpcHandler = std::function<nlohmann::json(const nlohmann::json& params, ClientConnection& client)>;

class GatewayServer {
public:
    GatewayServer(int port, std::shared_ptr<spdlog::logger> logger);
    ~GatewayServer();

    void start();
    void stop();
    bool is_running() const;

    void register_handler(const std::string& method, RpcHandler handler);
    void broadcast_event(const std::string& event, const nlohmann::json& payload);
    void send_event_to(const std::string& connection_id, const RpcEvent& event);

    int get_port() const { return port_; }
    size_t get_connection_count() const;
    int64_t get_uptime_seconds() const;

    // Configure authentication
    void set_auth(const std::string& mode, const std::string& token);
    const std::string& get_auth_mode() const { return auth_mode_; }

    // Set HTTP port for redirect when plain HTTP hits the WS port
    void set_http_redirect_port(int port) { http_redirect_port_ = port; }

private:
    void on_connection(std::shared_ptr<ix::ConnectionState> state,
                       ix::WebSocket& ws,
                       const ix::WebSocketMessagePtr& msg);
    void handle_message(const std::string& conn_id,
                        ix::WebSocket& ws,
                        const std::string& data);
    void handle_rpc_request(const std::string& conn_id,
                            ix::WebSocket& ws,
                            const RpcRequest& request);
    void send_challenge(ix::WebSocket& ws);
    bool handle_hello(const std::string& conn_id,
                      const nlohmann::json& params,
                      bool is_openclaw = false);

    int port_;
    std::shared_ptr<spdlog::logger> logger_;
    std::unique_ptr<ix::WebSocketServer> server_;
    std::atomic<bool> running_{false};
    int64_t start_time_ = 0;

    // Auth config
    std::string auth_mode_ = "token";
    std::string expected_token_;

    // HTTP redirect target (Control UI port)
    int http_redirect_port_ = 0;

    mutable std::mutex connections_mutex_;
    std::unordered_map<std::string, ClientConnection> connections_;
    std::unordered_map<std::string, ix::WebSocket*> ws_connections_;

    std::mutex handlers_mutex_;
    std::unordered_map<std::string, RpcHandler> handlers_;
};

} // namespace quantclaw::gateway
