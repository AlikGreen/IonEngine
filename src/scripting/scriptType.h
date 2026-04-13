#pragma once
#include "Coral/Type.hpp"

namespace ion
{
class ScriptType
{
public:
    explicit ScriptType(Coral::Type& type) : m_type(type) { }

    template<typename ... Args>
    void invokeStaticMethod(const std::string& name, Args&&... args)
    {
        m_type.InvokeStaticMethod(name, std::forward<Args>(args)...);
    }
private:
    Coral::Type& m_type;
};
}
