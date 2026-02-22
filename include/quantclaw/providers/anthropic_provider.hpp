#pragma once

#include "llm_provider.hpp"
#include <string>
#include <memory>
#include <spdlog/spdlog.h>

struct curl_slist;

namespace quantclaw {

class AnthropicProvider : public LLMProvider {
private:
    std::string api_key_;
    std::string base_url_;
    int timeout_;
    std::shared_ptr<spdlog::logger> logger_;

public:
    AnthropicProvider(const std::string& api_key,
                      const std::string& base_url,
                      int timeout,
                      std::shared_ptr<spdlog::logger> logger);

    ChatCompletionResponse chat_completion(const ChatCompletionRequest& request) override;
    void chat_completion_stream(const ChatCompletionRequest& request,
                                std::function<void(const ChatCompletionResponse&)> callback) override;
    std::string get_provider_name() const override;
    std::vector<std::string> get_supported_models() const override;

private:
    std::string make_api_request(const std::string& json_payload) const;
    curl_slist* create_headers() const;
};

} // namespace quantclaw
