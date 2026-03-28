#include "resourceFs.h"

#include <ranges>

namespace ion
{
    namespace fs = std::filesystem;

    static void insertSorted(std::vector<MountPoint>& vec, MountPoint mp)
    {
        const auto it = std::ranges::lower_bound(vec, mp.priority, std::greater{}, &MountPoint::priority);
        vec.insert(it, std::move(mp));
    }

    std::pair<std::string, fs::path> ResourceFS::splitPrefix(const std::string_view virtualPath)
    {
        const auto pos = virtualPath.find("://");
        if (pos == std::string_view::npos)
            return { "", fs::path(virtualPath) };

        return {
            std::string(virtualPath.substr(0, pos + 3)),
            fs::path(virtualPath.substr(pos + 3))
        };
    }

    void ResourceFS::mount(MountPoint mp)
    {
        const std::string prefix = mp.prefix;
        insertSorted(m_mounts[prefix], mp);

        if (!prefix.empty())
        {
            mp.prefix = "";
            insertSorted(m_mounts[""], std::move(mp));
        }
    }

    void ResourceFS::unmount(const std::string_view prefix)
    {
        m_mounts[std::string(prefix)] = {};
    }

    void ResourceFS::unmount(const std::string_view prefix, const fs::path& physicalRoot)
    {
        if (const auto it = m_mounts.find(std::string(prefix)); it != m_mounts.end())
        {
            auto& vec = it->second;
            std::erase_if(vec, [&](const MountPoint& mp) {
                return mp.physicalRoot == physicalRoot;
            });
            if (vec.empty()) m_mounts.erase(it);
        }
    }

    std::vector<MountPoint> ResourceFS::mounts() const
    {
        return m_mounts.at("");
    }

    fs::path ResourceFS::resolve(const std::string_view virtualPath) const
    {
        const auto [prefix, relative] = splitPrefix(virtualPath);

        if (const auto it = m_mounts.find(prefix); it != m_mounts.end())
            for (const auto& mount : it->second)
            {
                auto full = mount.physicalRoot / relative;
                if (fs::exists(full)) return full;
            }

        return {};
    }

    std::vector<fs::path> ResourceFS::resolveAll(const std::string_view virtualPath) const
    {
        const auto [prefix, relative] = splitPrefix(virtualPath);

        std::vector<fs::path> result;

        if (const auto it = m_mounts.find(prefix); it != m_mounts.end())
            for (const auto& mount : it->second)
            {
                auto full = mount.physicalRoot / relative;
                if (fs::exists(full)) result.push_back(full);
            }

        return result;
    }

    fs::path ResourceFS::resolveWrite(const std::string_view virtualPath, const MountAccess access) const
    {
        const auto [prefix, relative] = splitPrefix(virtualPath);

        if (const auto it = m_mounts.find(prefix); it != m_mounts.end())
            for (const auto& mount : it->second)
            {
                if(!hasFlag(mount.access, access)) continue;
                auto full = mount.physicalRoot / relative;
                if (fs::exists(full)) return full;
            }

        return {};
    }

    bool ResourceFS::exists(const std::string_view virtualPath) const
    {
        const auto [prefix, relative] = splitPrefix(virtualPath);

        if (const auto it = m_mounts.find(prefix); it != m_mounts.end())
            for (const auto& mount : it->second)
            {
                auto full = mount.physicalRoot / relative;
                if (fs::exists(full)) return true;
            }

        return false;
    }
}
