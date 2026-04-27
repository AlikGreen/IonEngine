#include "scriptComponentSerializer.h"

namespace ion
{
    enum class FieldType : uint8_t
    {
        Float = 0,
        Int,
        Bool,
        String,
        Vec2,
        Vec3,
    };

    std::optional<FieldType> fieldTypeTag(const FieldValue& value)
    {
        return std::visit([]<typename T>(const T& v) -> std::optional<FieldType>
{
            if constexpr (std::is_same_v<T, float>)        return FieldType::Float;
            if constexpr (std::is_same_v<T, int>)          return FieldType::Int;
            if constexpr (std::is_same_v<T, bool>)         return FieldType::Bool;
            if constexpr (std::is_same_v<T, std::string>)  return FieldType::String;
            if constexpr (std::is_same_v<T, glm::vec2>)    return FieldType::Vec2;
            if constexpr (std::is_same_v<T, glm::vec3>)    return FieldType::Vec3;
            return std::nullopt;
        }, value);
    }

    void ScriptComponentSerializer::serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps, const ScriptComponent &component)
    {
        assetStream.write<uint32_t>(component.scripts.size());


        for(auto& script : component.scripts)
        {
            assetStream.write(script.typeName());

            const size_t fieldCountCursor = assetStream.getCursor();
            assetStream.write<uint32_t>(0);

            uint32_t fieldCount = 0;
            for (const auto& [name, value] : script.fields())
            {
                auto tag = fieldTypeTag(value);
                if (!tag) continue;

                assetStream.write(name);
                assetStream.write<uint8_t>(static_cast<uint8_t>(*tag));

                std::visit([&](const auto& v) {
                    assetStream.write(v);
                }, value);

                fieldCount++;
            }

            assetStream.writeAt(fieldCountCursor, fieldCount);
        }
    }

    ScriptComponent ScriptComponentSerializer::deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry)
    {
        ScriptComponent component{};

        uint32_t scriptCount{};
        assetStream.read(scriptCount);

        for (size_t i = 0; i < scriptCount; i++)
        {
            std::string scriptType{};
            assetStream.read(scriptType);

            ScriptInstance script{scriptType};

            uint32_t fieldCount{};
            assetStream.read(fieldCount);

            for (size_t j = 0; j < fieldCount; j++)
            {
                std::string name{};
                assetStream.read(name);

                uint8_t rawTag{};
                assetStream.read(rawTag);
                const auto tag = static_cast<FieldType>(rawTag);

                switch (tag)
                {
                    case FieldType::Float:  { float v{};       assetStream.read(v); script.setField(name, v); break; }
                    case FieldType::Int:    { int v{};         assetStream.read(v); script.setField(name, v); break; }
                    case FieldType::Bool:   { bool v{};        assetStream.read(v); script.setField(name, v); break; }
                    case FieldType::String: { std::string v{}; assetStream.read(v); script.setField(name, v); break; }
                    case FieldType::Vec2:   { glm::vec2 v{};   assetStream.read(v); script.setField(name, v); break; }
                    case FieldType::Vec3:   { glm::vec3 v{};   assetStream.read(v); script.setField(name, v); break; }
                }
            }

            component.scripts.push_back(std::move(script));
        }

        return component;
    }
}
