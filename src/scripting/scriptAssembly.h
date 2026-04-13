#pragma once
#include <utility>

#include "Coral/Assembly.hpp"
#include "scriptType.h"

namespace ion
{
class ScriptAssembly
{
public:
    ScriptAssembly() = default;
    explicit ScriptAssembly(Coral::ManagedAssembly& assembly, std::filesystem::path path, std::string  name)
        : m_assembly(&assembly), m_name(std::move(name)), m_dllPath(std::move(path)) { }

    [[nodiscard]] Coral::ManagedAssembly& get() const { return *m_assembly; }
    void set(Coral::ManagedAssembly& assembly) { m_assembly = &assembly; }
    const std::string& name()    { return m_name; }
    std::filesystem::path dllPath()    { return m_dllPath; }

    void addInternalCall(const std::string& classPath, const std::string& method, void* funcPtr) const
    {
        m_assembly->AddInternalCall(classPath, method, funcPtr);
    }

    template<auto FuncPtr>
    void addInternalCall(const std::string& classPath, const std::string& method) const
    {
        m_assembly->AddInternalCall(classPath, method, reinterpret_cast<void*>(FuncPtr));
    }

    void uploadInternalCalls() const { m_assembly->UploadInternalCalls(); }

    [[nodiscard]] ScriptType getType(const std::string &name) const { return ScriptType(m_assembly->GetLocalType(name)); }
private:
    Coral::ManagedAssembly* m_assembly{};
    std::string m_name;
    std::filesystem::path m_dllPath;
};
}
