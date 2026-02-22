#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace quantclaw::cli {

class CLIManager {
public:
    struct Command {
        std::string name;
        std::string description;
        std::vector<std::string> aliases;
        std::function<int(int, char**)> handler;
    };
    
    CLIManager();
    
    void add_command(const Command& command);
    int run(int argc, char** argv);
    void show_help() const;
    
private:
    std::vector<Command> commands_;
};

} // namespace quantclaw::cli