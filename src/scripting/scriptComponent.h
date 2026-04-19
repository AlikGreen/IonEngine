#pragma once
#include <utility>

#include "scriptContext.h"
#include "Coral/ManagedObject.hpp"
#include "glm/glm.hpp"

namespace ion
{
using FieldValue = std::variant<float, int, bool, std::string, glm::vec2, glm::vec3>;

class ScriptInstance
{
public:
    ScriptInstance(std::string typeName, ScriptContext& scriptContext)
        : m_typeName(std::move(typeName)) { reload(scriptContext); }

    void saveState();
    void reload(ScriptContext& scriptContext);

    Coral::ManagedObject& object() { return m_object; }
private:
    std::string m_typeName;
    Coral::ManagedObject m_object;
    std::vector<std::pair<std::string, FieldValue>> m_fields;
};

struct ScriptComponent
{
    std::vector<ScriptInstance> scripts;

    void saveState();
    void reload(ScriptContext& scriptContext);
};
}
