#include "ion/platform.h"
#ifdef ION_PLATFORM_MAC

#include <mach-o/dyld.h>
#include <limits.h>
#include <unistd.h>

namespace ion::Platform
{
    std::filesystem::path getExecutablePath()
    {
        char path[PATH_MAX];
        uint32_t size = sizeof(path);
        _NSGetExecutablePath(path, &size);
        return std::filesystem::canonical(path);
    }

    void launch(const std::filesystem::path& executablePath, const std::string& args)
    {
        execl(executablePath.c_str(),
              executablePath.string().c_str(),
              args.c_str(),
              nullptr);
    }
}

#endif