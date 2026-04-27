#pragma once
#include "componentSerializer.h"
#include "scripting/scriptComponent.h"

namespace ion
{
class ScriptComponentSerializer final : public ComponentSerializer<ScriptComponent>
{
public:
    void serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps, const ScriptComponent &component) override;
    ScriptComponent deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry) override;
};
}
