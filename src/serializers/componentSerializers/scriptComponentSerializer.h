#pragma once
#include "componentSerializer.h"
#include "scripting/scriptComponent.h"

namespace ion
{
class ScriptComponentSerializer final : public ComponentSerializer<ScriptComponent>
{
public:
    static constexpr uint64_t typeId = grl::Hash::fnv1a64("ion::ScriptComponent");
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps, const ScriptComponent &component) override;
    ScriptComponent deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override;
};
}
