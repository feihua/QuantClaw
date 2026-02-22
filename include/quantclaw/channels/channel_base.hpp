#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace quantclaw {

struct Message {
    std::string id;
    std::string sender_id;
    std::string content;
    std::string channel_id;
    bool is_group_chat;
};

class Channel {
public:
    using MessageHandler = std::function<void(const Message&)>;
    
    virtual ~Channel() = default;
    
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void send_message(const std::string& channel_id, const std::string& message) = 0;
    virtual bool is_allowed(const std::string& sender_id) const = 0;
    virtual std::string get_channel_name() const = 0;
    
    void set_message_handler(MessageHandler handler) {
        message_handler_ = std::move(handler);
    }
    
protected:
    MessageHandler message_handler_;
};

} // namespace quantclaw