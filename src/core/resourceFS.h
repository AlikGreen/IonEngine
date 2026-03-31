#pragma once

#include <filesystem>

#include "enumFlags.h"

namespace ion
{
    enum class MountAccess : uint8_t
    {
        Read        = 0,
        WriteUser   = 1 << 0,
        WriteEditor = 1 << 1,
        WriteEngine = 1 << 2,

        UserFull    = WriteUser | WriteEditor,
        EditorFull  = WriteEditor | WriteEngine,
    };

    ION_DEFINE_ENUM_FLAGS(MountAccess);

    enum class MountPriority
    {
        Engine  = 1,
        Package = 100,
        Editor  = 200,
        Project = 300,
    };

    struct MountPoint
    {
        std::string name;
        std::string prefix;
        std::filesystem::path physicalRoot;
        int         priority;
        MountAccess access = MountAccess::Read;
    };

    class ResourceFS
    {
    public:
        ResourceFS() = default;

        ResourceFS(const ResourceFS&) = delete;
        ResourceFS& operator=(const ResourceFS&) = delete;

        void mount(MountPoint mp);

        void unmount(std::string_view prefix);
        void unmount(std::string_view prefix, const std::filesystem::path &physicalRoot);

        [[nodiscard]] std::vector<MountPoint> mounts() const;
        [[nodiscard]] std::filesystem::path resolve(std::string_view virtualPath) const;
        [[nodiscard]] std::vector<std::filesystem::path> resolveAll(std::string_view virtualPath) const;
        [[nodiscard]] std::filesystem::path resolveWrite(std::string_view virtualPath, MountAccess access) const;
        [[nodiscard]] bool exists(std::string_view virtualPath) const;

        static std::pair<std::string, std::filesystem::path> splitPrefix(std::string_view virtualPath);
    private:
        std::unordered_map<std::string, std::vector<MountPoint>> m_mounts;
    };

}
