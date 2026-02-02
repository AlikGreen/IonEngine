#include "assetsWindow.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <util/file.h>

#include "propertiesWindow.h"
#include "../editorAssetImporter.h"
#include "asset/assetManager.h"
#include "graphics/imGuiSystem.h"
#include "../editorSystem.h"
#include "../events/inspectEvent.h"
#include "core/eventManager.h"
#include "core/scene.h"
#include "graphics/events/dropFileEvent.h"

struct ImRect;

namespace Neon::Editor
{
    void AssetsWindow::consumeFileDropInRect(const ImRect& rect)
    {
        AssetManager& assetManager = Engine::getAssetManager();

        for (const auto& path : droppedPaths)
        {
            const ImVec2 p = ImGui::GetMousePos();

            if (rect.Contains(p))
            {
                EditorAssetImporter::import(path, currentPath);
            }
        }

        droppedPaths.clear();
    }


    void AssetsWindow::render()
    {
        AssetManager& assetManager = Engine::getAssetManager();
        static AssetID selectedAssetId = AssetID::invalid();

        static AssetID hoveredAssetId = AssetID::invalid();
        static double hoverStartTime = 0.0;

        ImGui::Begin("Assets");
        ImGui::Separator();

        ImGui::BeginChild("##assetGrid", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);

        {
            const ImVec2 winPos = ImGui::GetWindowPos();
            const ImVec2 winSize = ImGui::GetWindowSize();
            const ImRect dropRect(winPos, ImVec2(winPos.x + winSize.x, winPos.y + winSize.y));
            consumeFileDropInRect(dropRect);
        }

        const auto assetHandles = assetManager.getAllAssetIDs();
        if (assetHandles.empty())
        {
            ImGui::TextDisabled("No assets found.");
            ImGui::TextDisabled("Drag .glb files here to import.");
            ImGui::EndChild();
            return;
        }

        constexpr float padding = 8.0f;
        constexpr float tileSize = 96.0f;

        const float textHeight = ImGui::GetTextLineHeight();
        constexpr float spacingY = 6.0f;
        const float tileHeight = tileSize + spacingY + textHeight + 6.0f;

        const float availX = ImGui::GetContentRegionAvail().x;
        int columns = static_cast<int>((availX + padding) / (tileSize + padding));
        if (columns < 1)
        {
            columns = 1;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(padding * 0.5f, padding * 0.5f));
        ImGui::PushFont(ImGuiSystem::smallFont);

        if (ImGui::BeginTable("##assetGridTable", columns, ImGuiTableFlags_SizingFixedFit))
        {
            for (AssetID assetId : assetHandles)
            {
                const AssetMetadata assetMetadata = assetManager.getMetadata(assetId);
                const std::string assetName = assetMetadata.name;

                ImGui::TableNextColumn();
                ImGui::PushID(static_cast<int>(static_cast<size_t>(assetId)));

                const bool isSelected = (selectedAssetId == assetId);

                ImGui::InvisibleButton("##tile", ImVec2(tileSize, tileHeight));

                const bool isHovered = ImGui::IsItemHovered();
                const bool isClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
                const bool isReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

                if (isClicked)
                {
                    selectedAssetId = assetId;
                }

                if(isReleased && isHovered && !ImGui::GetDragDropPayload())
                    Engine::getEventManager().queueEvent<InspectEvent>(assetId);

                const ImVec2 rectMin = ImGui::GetItemRectMin();
                const ImVec2 rectMax = ImGui::GetItemRectMax();

                ImDrawList* drawList = ImGui::GetWindowDrawList();

                const ImU32 bgCol = ImGui::GetColorU32((isSelected || isHovered) ? ImGuiCol_FrameBgHovered : ImGuiCol_WindowBg);
                const ImU32 borderCol = ImGui::GetColorU32(ImGuiCol_Border);

                drawList->AddRectFilled(rectMin, rectMax, bgCol, 4.0f);
                drawList->AddRect(rectMin, rectMax, borderCol, 4.0f);

                const auto imgMin = ImVec2(rectMin.x + 3.0f, rectMin.y + 3.0f);
                const auto imgMax = ImVec2(imgMin.x + tileSize - 6.0f, imgMin.y + tileSize - 6.0f);

                // ImTextureID previewTex = assetManager.getPreviewTexture(assetId);
                // if (previewTex != nullptr)
                // {
                //     drawList->AddImage(previewTex, imgMin, imgMax);
                // }
                // else
                // {
                    drawList->AddRectFilled(imgMin, imgMax, ImGui::GetColorU32(ImGuiCol_FrameBg), 2.0f);
                // }

                const ImVec2 textPos = ImVec2(rectMin.x + 4.0f, imgMax.y + spacingY);
                const float textMaxX = rectMax.x - 4.0f;

                const float nameAvail = textMaxX - textPos.x;
                const bool isNameTruncated = (ImGui::CalcTextSize(assetName.c_str()).x > nameAvail);

                ImGui::PushClipRect(rectMin, rectMax, true);
                drawList->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), assetName.c_str());
                ImGui::PopClipRect();

                if (isHovered)
                {
                    const double now = ImGui::GetTime();
                    if (hoveredAssetId != assetId)
                    {
                        hoveredAssetId = assetId;
                        hoverStartTime = now;
                    }

                    if (isNameTruncated && (now - hoverStartTime) >= 1.0)
                    {
                        ImGui::BeginTooltip();
                        ImGui::TextUnformatted(assetName.c_str());
                        ImGui::EndTooltip();
                    }
                }
                else if (hoveredAssetId == assetId)
                {
                    hoveredAssetId = AssetID::invalid();
                }

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                {
                    ImGui::SetDragDropPayload("ASSET_ID", &assetId, sizeof(assetId));
                    ImGui::TextUnformatted(assetName.c_str());
                    ImGui::EndDragDropSource();
                }

                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Delete"))
                    {
                        Log::error("Deleting assets not implemented");
                    }
                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
        ImGui::PopFont();

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::End();
    }

    void AssetsWindow::event(Event *event)
    {
        if(const auto* windowEvent = dynamic_cast<DropFileEvent*>(event))
        {
            droppedPaths.emplace_back(windowEvent->getPath());
            Log::info("Dropped file: {}", windowEvent->getPath());
        }
    }
}
