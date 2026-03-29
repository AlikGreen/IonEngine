#include "consoleWindowSink.h"

#include "imGuiSystem.h"
#include "core/engine.h"

namespace ion
{
    void ConsoleWindowSink::handle(const std::string_view msg, const clogr::Level level, const std::string_view loggerName, const clogr::Pattern &pattern)
    {
        Engine::getSystem<ImGuiSystem>()->onMessage({std::string(msg), pattern.format(msg, loggerName, level), level});
    }

    bool ConsoleWindowSink::shouldLog(clogr::Level level)
    {
        return true;
    }
}
