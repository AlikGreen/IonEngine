#include "scriptComponent.h"

namespace ion
{
    void ScriptInstance::setField(const std::string &name, FieldValue value)
    {
        m_fields[name] = std::move(value);

        if (m_loaded)
            applyField(name, m_fields[name]);
    }

    const FieldValue * ScriptInstance::getField(const std::string &name) const
    {
        const auto it = m_fields.find(name);
        return it != m_fields.end() ? &it->second : nullptr;
    }

    void ScriptInstance::unload()
    {
        if (!m_loaded) return;

        syncFieldsFromObject();

        m_object.Destroy();
        m_loaded = false;
    }

    void ScriptInstance::reload(ScriptContext &scriptContext, entis::Entity entity)
    {
        if (m_loaded) unload();

        m_type = *scriptContext.findType(m_typeName);
        m_object = m_type.CreateInstance(entity.id(), &entity.registry());
        m_loaded = true;

        for (auto& field : m_type.GetFields())
        {
            if (field.GetAccessibility() != Coral::TypeAccessibility::Public) continue;
            if (m_fields.contains(field.GetName())) continue;

            m_fields[field.GetName()] = fieldDefaultValue(field);
        }

        for (const auto& [name, value] : m_fields)
            applyField(name, value);

    }

    void ScriptInstance::syncFieldsFromObject() const
    {
        for (auto& [name, value] : m_fields)
        {
            std::visit([&]<typename T>(T& v)
            {
                v = m_object.GetFieldValue<T>(name);
            }, value);
        }
    }

    void ScriptInstance::syncFieldsToObject() const
    {
        for (auto& [name, value] : m_fields)
        {
            std::visit([&]<typename T>(const T& v)
            {
                m_object.SetFieldValue<T>(name, v);
            }, value);
        }
    }

    FieldValue ScriptInstance::fieldDefaultValue(Coral::FieldInfo &field)
    {
        const auto typeName = field.GetType().GetFullName();

        if (typeName == "System.Single")  return float{};
        if (typeName == "System.Int32")   return int{};
        if (typeName == "System.Boolean") return bool{};
        if (typeName == "System.String")  return std::string{};
        if (typeName == "IonEngine.Maths.Vector2") return glm::vec2{};
        if (typeName == "IonEngine.Maths.Vector3") return glm::vec3{};

        return float{};
    }

    void ScriptInstance::applyField(const std::string &name, const FieldValue &value)
    {
        std::visit([&](const auto& v)
        {
            m_object.SetFieldValue(name, v);
        }, value);
    }

    void ScriptComponent::unload()
    {
        for(auto& script : scripts)
        {
            script.unload();
        }
    }

    void ScriptComponent::reload(ScriptContext &scriptContext, const entis::Entity entity)
    {
        for(auto& script : scripts)
        {
            script.reload(scriptContext, entity);
        }
    }

    ScriptComponent::ScriptComponent(const ScriptComponent &other)
    {
        for (const auto& script : other.scripts)
        {
            auto copy = ScriptInstance(script.typeName());
            for (const auto& [name, value] : script.fields())
                copy.setField(name, value);

            scripts.push_back(std::move(copy));
        }
    }

    ScriptComponent & ScriptComponent::operator=(const ScriptComponent &other)
    {
        if (this == &other) return *this;
        scripts.clear();
        for (const auto& script : other.scripts)
        {
            ScriptInstance copy{script.typeName()};
            for (const auto& [name, value] : script.fields())
                copy.setField(name, value);
            scripts.push_back(std::move(copy));
        }
        return *this;
    }
}
