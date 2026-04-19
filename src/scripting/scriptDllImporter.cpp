#include "scriptDllImporter.h"

namespace ion
{
    std::vector<std::byte> readFileBytes(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if(!file)
            return {};

        const std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<std::byte> buffer(size);
        if(size > 0)
            file.read(reinterpret_cast<char *>(buffer.data()), size);

        return buffer;
    }

    grl::Box<ScriptAssemblyData> ScriptDllImporter::import(const std::filesystem::path &src, const NoOptions &options)
    {
        auto bytes = readFileBytes(src);
        return grl::makeBox<ScriptAssemblyData>(bytes);
    }

    bool ScriptDllImporter::canImport(const std::filesystem::path &src) const
    {
        return src.extension() == ".dll";
    }
}
