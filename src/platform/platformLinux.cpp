#include "ion/platform.h"
#ifdef ION_PLATFORM_LINUX

#include <unistd.h>
#include <limits.h>


namespace ion::Platform
{
    std::filesystem::path getExecutablePath()
    {
        char path[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
        return std::filesystem::path(std::string(path, count > 0 ? count : 0));
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