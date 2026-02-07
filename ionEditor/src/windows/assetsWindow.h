#pragma once
#include <imgui_internal.h>
#include <vector>

#include "editorWindow.h"
#include "../editorFileSystem.h"
#include "asset/assetManager.h"

namespace ion::Editor
{
class AssetsWindow final : public EditorWindow
{
public:
    void startup() override;
    void render() override;
    void event(Event *event) override;
private:
    void drawCurrentDir();
    void drawAssetTile(AssetId assetId);
    void drawFolderTile(const grl::Rc<VFSNode>& folder);

    const float padding = 8.0f;
    const float tileSize = 96.0f;
    const float spacingY = 6.0f;
    float textHeight = 10.0f;
    float tileHeight = 10.0f;

    grl::Rc<VFSNode> currentDir = nullptr;
    EditorFileSystem fileSystem;
    std::vector<std::string> droppedPaths;
    AssetId hoveredAssetId = AssetId::invalid();
    AssetId selectedAssetId = AssetId::invalid();
    double hoverStartTime = 0.0;

    void consumeFileDropInRect(const ImRect &rect);
};
}
