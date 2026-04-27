#pragma once
#include <unordered_set>
#include <utility>
#include <entis/entis.h>

#include "scriptContext.h"
#include "Coral/ManagedObject.hpp"
#include "glm/glm.hpp"

namespace ion
{
using FieldValue = std::variant<float, int, bool, std::string, glm::vec2, glm::vec3>;

class ScriptInstance
{
public:
    ScriptInstance() = default;
    explicit ScriptInstance(std::string typeName)
        : m_typeName(std::move(typeName)) { }

    ScriptInstance(const ScriptInstance&)            = delete;
    ScriptInstance& operator=(const ScriptInstance&) = delete;

    ScriptInstance(ScriptInstance&&)            = default;
    ScriptInstance& operator=(ScriptInstance&&) = default;

    void setField(const std::string& name, FieldValue value);
    [[nodiscard]] const FieldValue* getField(const std::string& name) const;

    [[nodiscard]] bool isLoaded()                       const { return m_loaded; }
    [[nodiscard]] std::string typeName()                const { return m_typeName; }
    [[nodiscard]] Coral::Type type()                    const { return m_type; }
    Coral::ManagedObject& object()                  { return m_object; }
    [[nodiscard]] const Coral::ManagedObject& object()  const { return m_object; }
    [[nodiscard]] const  std::unordered_map<std::string, FieldValue>& fields() const
    {
        if (isLoaded())
            syncFieldsFromObject();

        return m_fields;
    }

    void syncFieldsFromObject() const;
    void syncFieldsToObject() const;
private:
    friend class ScriptComponent;

    void unload();
    void reload(ScriptContext& scriptContext, entis::Entity entity);

    static FieldValue fieldDefaultValue(Coral::FieldInfo& field);
    void applyField(const std::string& name, const FieldValue& value);

    std::string              m_typeName;
    Coral::ManagedObject     m_object;
    Coral::Type              m_type;
    bool                     m_loaded = false;

    mutable std::unordered_map<std::string, FieldValue> m_fields;
};

struct ScriptComponent
{
    ScriptComponent() = default;

    ScriptComponent(const ScriptComponent& other);
    ScriptComponent& operator=(const ScriptComponent& other);


    std::vector<ScriptInstance> scripts;

    void unload();
    void reload(ScriptContext& scriptContext, entis::Entity entity);
};
}
