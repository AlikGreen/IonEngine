#pragma once

#include "Coral/Assembly.hpp"

namespace ion
{
class CallBinder
{
public:
    struct ScopedTypeBinder;

    explicit CallBinder(Coral::ManagedAssembly& assembly)
        : m_assembly(assembly) {}

    template<auto Fn>
    void bind(const std::string_view typeName, const std::string_view methodName) const
    {
        m_assembly.AddInternalCall(
            std::string(typeName),
            std::string(methodName),
            reinterpret_cast<void*>(Fn)
        );
    }
private:
    Coral::ManagedAssembly& m_assembly;
};

class ScriptBindings
{
public:
    virtual ~ScriptBindings() = default;
    virtual void registerCalls(CallBinder& binder) = 0;
};
}
