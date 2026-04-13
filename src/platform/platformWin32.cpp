#include "ion/platform.h"

#ifdef ION_PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <filesystem>

namespace ion::Platform
{
    std::filesystem::path getExecutablePath()
    {
        char path[MAX_PATH];
        GetModuleFileNameA(nullptr, path, MAX_PATH);
        return {path};
    }

    void launch(const std::filesystem::path& executablePath, const std::string& args)
    {
        ShellExecuteA(nullptr, "open",
                      executablePath.string().c_str(),
                      args.c_str(),
                      nullptr, SW_SHOW);
    }
}

#endif