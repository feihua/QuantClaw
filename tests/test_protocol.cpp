#include <gtest/gtest.h>
#include "quantclaw/gateway/protocol.hpp"

using namespace quantclaw::gateway;

// --- Frame type conversion ---

TEST(ProtocolTest, FrameTypeToString) {
    EXPECT_EQ(frame_type_to_string(FrameType::Request), "req");
    EXPECT_EQ(frame_type_to_string(FrameType::Response), "res");
    EXPECT_EQ(frame_type_to_string(FrameType::Event), "event");
}

TEST(ProtocolTest, FrameTypeFromString) {
    EXPECT_EQ(frame_type_from_string("req"), FrameType::Request);
    EXPECT_EQ(frame_type_from_string("res"), FrameType::Response);
    EXPECT_EQ(frame_type_from_string("event"), FrameType::Event);
    EXPECT_THROW(frame_type_from_string("invalid"), std::runtime_error);
}

// --- RpcRequest ---

TEST(ProtocolTest, RpcRequestToJson) {
    RpcRequest req;
    req.id = "42";
    req.method = "gateway.health";
    req.params = {{"key", "value"}};

    auto j = req.to_json();
    EXPECT_EQ(j["type"], "req");
    EXPECT_EQ(j["id"], "42");
    EXPECT_EQ(j["method"], "gateway.health");
    EXPECT_EQ(j["params"]["key"], "value");
}

TEST(ProtocolTest, RpcRequestFromJson) {
    nlohmann::json j = {
        {"type", "req"},
        {"id", "7"},
        {"method", "sessions.list"},
        {"params", {{"limit", 10}}}
    };

    auto req = RpcRequest::from_json(j);
    EXPECT_EQ(req.id, "7");
    EXPECT_EQ(req.method, "sessions.list");
    EXPECT_EQ(req.params["limit"], 10);
}

TEST(ProtocolTest, RpcRequestFromJsonNoParams) {
    nlohmann::json j = {
        {"type", "req"},
        {"id", "1"},
        {"method", "gateway.health"}
    };

    auto req = RpcRequest::from_json(j);
    EXPECT_EQ(req.method, "gateway.health");
    EXPECT_TRUE(req.params.is_object());
    EXPECT_TRUE(req.params.empty());
}

// --- RpcResponse ---

TEST(ProtocolTest, RpcResponseSuccess) {
    auto resp = RpcResponse::success("42", {{"status", "ok"}});

    EXPECT_EQ(resp.id, "42");
    EXPECT_TRUE(resp.ok);
    EXPECT_EQ(resp.payload["status"], "ok");

    auto j = resp.to_json();
    EXPECT_EQ(j["type"], "res");
    EXPECT_EQ(j["id"], "42");
    EXPECT_TRUE(j["ok"]);
    EXPECT_EQ(j["payload"]["status"], "ok");
    EXPECT_FALSE(j.contains("error"));
}

TEST(ProtocolTest, RpcResponseFailure) {
    auto resp = RpcResponse::failure("99", "Not found");

    EXPECT_EQ(resp.id, "99");
    EXPECT_FALSE(resp.ok);
    EXPECT_EQ(resp.error, "Not found");

    auto j = resp.to_json();
    EXPECT_EQ(j["type"], "res");
    EXPECT_FALSE(j["ok"]);
    EXPECT_EQ(j["error"], "Not found");
    EXPECT_FALSE(j.contains("payload"));
}

// --- RpcEvent ---

TEST(ProtocolTest, RpcEventToJson) {
    RpcEvent evt;
    evt.event = "agent.text_delta";
    evt.payload = {{"text", "hello"}};
    evt.seq = 5;

    auto j = evt.to_json();
    EXPECT_EQ(j["type"], "event");
    EXPECT_EQ(j["event"], "agent.text_delta");
    EXPECT_EQ(j["payload"]["text"], "hello");
    EXPECT_EQ(j["seq"], 5);
    EXPECT_FALSE(j.contains("stateVersion"));
}

TEST(ProtocolTest, RpcEventWithStateVersion) {
    RpcEvent evt;
    evt.event = "gateway.tick";
    evt.payload = {};
    evt.state_version = 12;

    auto j = evt.to_json();
    EXPECT_EQ(j["stateVersion"], 12);
}

TEST(ProtocolTest, RpcEventNoOptionals) {
    RpcEvent evt;
    evt.event = "test";
    evt.payload = {};

    auto j = evt.to_json();
    EXPECT_FALSE(j.contains("seq"));
    EXPECT_FALSE(j.contains("stateVersion"));
}

// --- ConnectChallenge ---

TEST(ProtocolTest, ConnectChallengeToJson) {
    ConnectChallenge challenge;
    challenge.nonce = "abc123";
    challenge.timestamp = 1700000000;

    auto j = challenge.to_json();
    EXPECT_EQ(j["type"], "event");
    EXPECT_EQ(j["event"], "connect.challenge");
    EXPECT_EQ(j["payload"]["nonce"], "abc123");
    EXPECT_EQ(j["payload"]["ts"], 1700000000);
}

// --- ConnectHelloParams ---

TEST(ProtocolTest, ConnectHelloParamsFromJson) {
    nlohmann::json j = {
        {"minProtocol", 1},
        {"maxProtocol", 2},
        {"clientName", "test-client"},
        {"clientVersion", "1.0.0"},
        {"role", "operator"},
        {"scopes", {"operator.read", "operator.write"}},
        {"authToken", "secret"},
        {"deviceId", "dev-001"}
    };

    auto params = ConnectHelloParams::from_json(j);
    EXPECT_EQ(params.min_protocol, 1);
    EXPECT_EQ(params.max_protocol, 2);
    EXPECT_EQ(params.client_name, "test-client");
    EXPECT_EQ(params.client_version, "1.0.0");
    EXPECT_EQ(params.role, "operator");
    EXPECT_EQ(params.scopes.size(), 2u);
    EXPECT_EQ(params.auth_token, "secret");
    EXPECT_EQ(params.device_id, "dev-001");
}

TEST(ProtocolTest, ConnectHelloParamsDefaults) {
    auto params = ConnectHelloParams::from_json(nlohmann::json::object());
    EXPECT_EQ(params.min_protocol, 1);
    EXPECT_EQ(params.max_protocol, 1);
    EXPECT_EQ(params.role, "operator");
    EXPECT_EQ(params.scopes.size(), 2u);
}

// --- HelloOkPayload ---

TEST(ProtocolTest, HelloOkPayloadToJson) {
    HelloOkPayload payload;
    payload.protocol = 1;
    payload.policy = "permissive";
    payload.authenticated = true;
    payload.tick_interval_ms = 15000;

    auto j = payload.to_json();
    EXPECT_EQ(j["protocol"], 1);
    EXPECT_EQ(j["policy"], "permissive");
    EXPECT_TRUE(j["authenticated"]);
    EXPECT_EQ(j["tickIntervalMs"], 15000);
}

// --- parse_frame_type ---

TEST(ProtocolTest, ParseFrameType) {
    EXPECT_EQ(parse_frame_type({{"type", "req"}}), FrameType::Request);
    EXPECT_EQ(parse_frame_type({{"type", "res"}}), FrameType::Response);
    EXPECT_EQ(parse_frame_type({{"type", "event"}}), FrameType::Event);
}

// --- Method / Event constants ---

TEST(ProtocolTest, MethodConstants) {
    EXPECT_STREQ(methods::CONNECT_HELLO, "connect.hello");
    EXPECT_STREQ(methods::GATEWAY_HEALTH, "gateway.health");
    EXPECT_STREQ(methods::GATEWAY_STATUS, "gateway.status");
    EXPECT_STREQ(methods::CONFIG_GET, "config.get");
    EXPECT_STREQ(methods::AGENT_REQUEST, "agent.request");
    EXPECT_STREQ(methods::AGENT_STOP, "agent.stop");
    EXPECT_STREQ(methods::SESSIONS_LIST, "sessions.list");
    EXPECT_STREQ(methods::SESSIONS_HISTORY, "sessions.history");
    EXPECT_STREQ(methods::CHANNELS_LIST, "channels.list");
}

TEST(ProtocolTest, EventConstants) {
    EXPECT_STREQ(events::CONNECT_CHALLENGE, "connect.challenge");
    EXPECT_STREQ(events::TEXT_DELTA, "agent.text_delta");
    EXPECT_STREQ(events::TOOL_USE, "agent.tool_use");
    EXPECT_STREQ(events::TOOL_RESULT, "agent.tool_result");
    EXPECT_STREQ(events::MESSAGE_END, "agent.message_end");
    EXPECT_STREQ(events::TICK, "gateway.tick");
}

// --- Roundtrip serialization ---

TEST(ProtocolTest, RequestRoundtrip) {
    RpcRequest original;
    original.id = "test-id";
    original.method = "test.method";
    original.params = {{"foo", "bar"}, {"num", 42}};

    auto j = original.to_json();
    auto parsed = RpcRequest::from_json(j);

    EXPECT_EQ(parsed.id, original.id);
    EXPECT_EQ(parsed.method, original.method);
    EXPECT_EQ(parsed.params, original.params);
}
