#pragma once

namespace ion
{
class ConsoleWindowSink final : public clogr::Sink
{
public:
    void handle(std::string_view msg, clogr::Level level, std::string_view loggerName, const clogr::Pattern &pattern) override;
    bool shouldLog(clogr::Level level) override;
};
}
