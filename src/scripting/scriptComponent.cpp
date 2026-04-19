#include "scriptComponent.h"

namespace ion
{
    void ScriptInstance::saveState()
    {
        for(auto field : m_object.GetType().GetFields())
        {
            const std::string name     = field.GetName();
            const auto typeName = field.GetType().GetFullName();
            FieldValue value;

            if     (typeName == "System.Single")  value = m_object.GetFieldValue<float>(name);
            else if(typeName == "System.Int32")   value = m_object.GetFieldValue<int>(name);
            else if(typeName == "System.Boolean") value = m_object.GetFieldValue<bool>(name);
            else if(typeName == "System.String")  value = m_object.GetFieldValue<std::string>(name);

            m_fields.emplace_back(name, value);
        }
    }

    void ScriptInstance::reload(ScriptContext &scriptContext)
    {
        const auto type = scriptContext.findType(m_typeName);
        m_object = type->CreateInstance();

        for(auto& [name, value] : m_fields)
        {
            std::visit([&](auto&& v)
            {
                m_object.SetFieldValue(name, v);
            }, value);
        }
    }

    void ScriptComponent::saveState()
    {
        for(auto& script : scripts)
        {
            script.saveState();
        }
    }

    void ScriptComponent::reload(ScriptContext &scriptContext)
    {
        for(auto& script : scripts)
        {
            script.reload(scriptContext);
        }
    }
}
