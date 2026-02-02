#pragma once
#include <imgui_internal.h>
#include <vector>

#include "editorWindow.h"
#include "asset/assetManager.h"

namespace Neon::Editor
{
class AssetsWindow final : public EditorWindow
{
public:
    void render() override;
    void event(Event *event) override;
private:
    std::string currentPath = "";
    std::vector<std::string> droppedPaths;
    AssetID hoveredAssetId = AssetID::invalid();
    double hoverStartTime = 0.0;

    void consumeFileDropInRect(const ImRect &rect);
};
}
