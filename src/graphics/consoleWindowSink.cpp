#include "consoleWindowSink.h"

#include "imGuiSystem.h"
#include "core/engine.h"

namespace ion
{
    void ConsoleWindowSink::handle(const std::string_view msg, const clogr::Level level, const std::string_view loggerName, const clogr::Pattern &pattern)
    {
        Engine::getSystem<ImGuiSystem>()->onMessage(level, pattern.format(msg, loggerName, level));
    }

    bool ConsoleWindowSink::shouldLog(clogr::Level level)
    {
        return true;
    }
}
