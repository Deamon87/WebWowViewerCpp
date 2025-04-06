
#include <string>

class IMessLogger {
public:
    enum class MessageType : int {
        INFO, ERROR, WARNING, DEBUG
    };

    virtual void log(const std::string &message) = 0;
};