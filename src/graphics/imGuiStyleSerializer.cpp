#include "imGuiStyleSerializer.h"

namespace jc = jsoncons;

namespace ion
{
    jsoncons::json ImGuiStyleSerializer::serialize(const ImGuiStyle& style)
    {
        jc::json j;
        j["FontSizeBase"] = style.FontSizeBase;
        j["FontScaleMain"] = style.FontScaleMain;
        j["FontScaleDpi"] = style.FontScaleDpi;

        j["Alpha"] = style.Alpha;
        j["DisabledAlpha"] = style.DisabledAlpha;

        j["WindowPadding"] = jc::json::array{ style.WindowPadding.x, style.WindowPadding.y };
        j["WindowRounding"] = style.WindowRounding;
        j["WindowBorderSize"] = style.WindowBorderSize;
        j["WindowBorderHoverPadding"] = style.WindowBorderHoverPadding;
        j["WindowMinSize"] = jc::json::array{ style.WindowMinSize.x, style.WindowMinSize.y };
        j["WindowTitleAlign"] = jc::json::array{ style.WindowTitleAlign.x, style.WindowTitleAlign.y };
        j["WindowMenuButtonPosition"] = style.WindowMenuButtonPosition;

        j["ChildRounding"] = style.ChildRounding;
        j["ChildBorderSize"] = style.ChildBorderSize;

        j["PopupRounding"] = style.PopupRounding;
        j["PopupBorderSize"] = style.PopupBorderSize;

        j["FramePadding"] = jc::json::array{ style.FramePadding.x, style.FramePadding.y };
        j["FrameRounding"] = style.FrameRounding;
        j["FrameBorderSize"] = style.FrameBorderSize;

        j["ItemSpacing"] = jc::json::array{ style.ItemSpacing.x, style.ItemSpacing.y };
        j["ItemInnerSpacing"] = jc::json::array{ style.ItemInnerSpacing.x, style.ItemInnerSpacing.y };
        j["CellPadding"] = jc::json::array{ style.CellPadding.x, style.CellPadding.y };
        j["TouchExtraPadding"] = jc::json::array{ style.TouchExtraPadding.x, style.TouchExtraPadding.y };

        j["IndentSpacing"] = style.IndentSpacing;
        j["ColumnsMinSpacing"] = style.ColumnsMinSpacing;

        j["ScrollbarSize"] = style.ScrollbarSize;
        j["ScrollbarRounding"] = style.ScrollbarRounding;
        j["ScrollbarPadding"] = style.ScrollbarPadding;

        j["GrabMinSize"] = style.GrabMinSize;
        j["GrabRounding"] = style.GrabRounding;

        j["LogSliderDeadzone"] = style.LogSliderDeadzone;
        j["ImageBorderSize"] = style.ImageBorderSize;

        j["TabRounding"] = style.TabRounding;
        j["TabBorderSize"] = style.TabBorderSize;
        j["TabMinWidthBase"] = style.TabMinWidthBase;
        j["TabMinWidthShrink"] = style.TabMinWidthShrink;
        j["TabCloseButtonMinWidthSelected"] = style.TabCloseButtonMinWidthSelected;
        j["TabCloseButtonMinWidthUnselected"] = style.TabCloseButtonMinWidthUnselected;

        j["TabBarBorderSize"] = style.TabBarBorderSize;
        j["TabBarOverlineSize"] = style.TabBarOverlineSize;

        j["TableAngledHeadersAngle"] = style.TableAngledHeadersAngle;
        j["TableAngledHeadersTextAlign"] = jc::json::array{ style.TableAngledHeadersTextAlign.x, style.TableAngledHeadersTextAlign.y };

        j["TreeLinesFlags"] = style.TreeLinesFlags;
        j["TreeLinesSize"] = style.TreeLinesSize;
        j["TreeLinesRounding"] = style.TreeLinesRounding;

        j["DragDropTargetRounding"] = style.DragDropTargetRounding;
        j["DragDropTargetBorderSize"] = style.DragDropTargetBorderSize;
        j["DragDropTargetPadding"] = style.DragDropTargetPadding;

        j["ColorMarkerSize"] = style.ColorMarkerSize;
        j["ColorButtonPosition"] = style.ColorButtonPosition;

        j["ButtonTextAlign"] = jc::json::array{ style.ButtonTextAlign.x, style.ButtonTextAlign.y };
        j["SelectableTextAlign"] = jc::json::array{ style.SelectableTextAlign.x, style.SelectableTextAlign.y };

        j["SeparatorTextBorderSize"] = style.SeparatorTextBorderSize;
        j["SeparatorTextAlign"] = jc::json::array{ style.SeparatorTextAlign.x, style.SeparatorTextAlign.y };
        j["SeparatorTextPadding"] = jc::json::array{ style.SeparatorTextPadding.x, style.SeparatorTextPadding.y };

        j["DisplayWindowPadding"] = jc::json::array{ style.DisplayWindowPadding.x, style.DisplayWindowPadding.y };
        j["DisplaySafeAreaPadding"] = jc::json::array{ style.DisplaySafeAreaPadding.x, style.DisplaySafeAreaPadding.y };

        j["DockingNodeHasCloseButton"] = style.DockingNodeHasCloseButton;
        j["DockingSeparatorSize"] = style.DockingSeparatorSize;

        j["MouseCursorScale"] = style.MouseCursorScale;

        j["AntiAliasedLines"] = style.AntiAliasedLines;
        j["AntiAliasedLinesUseTex"] = style.AntiAliasedLinesUseTex;
        j["AntiAliasedFill"] = style.AntiAliasedFill;

        j["CurveTessellationTol"] = style.CurveTessellationTol;
        j["CircleTessellationMaxError"] = style.CircleTessellationMaxError;

        j["HoverStationaryDelay"] = style.HoverStationaryDelay;
        j["HoverDelayShort"] = style.HoverDelayShort;
        j["HoverDelayNormal"] = style.HoverDelayNormal;

        j["HoverFlagsForTooltipMouse"] = style.HoverFlagsForTooltipMouse;
        j["HoverFlagsForTooltipNav"] = style.HoverFlagsForTooltipNav;

        j["_MainScale"] = style._MainScale;
        j["_NextFrameFontSizeBase"] = style._NextFrameFontSizeBase;

        jc::json colorsJson = jc::json::array();

        for (auto color : style.Colors)
        {
            colorsJson.push_back(jc::json::array{ color.x, color.y, color.z, color.w} );
        }

        j["Colors"] = colorsJson;

        return j;
    }

    void ImGuiStyleSerializer::serialize(const std::filesystem::path &path, const ImGuiStyle &style)
    {
        const auto json = serialize(style);
        std::ostringstream outStrStream;
        outStrStream << jc::pretty_print(json);
        std::string prettyString = outStrStream.str();
        grl::File::write(path.string(), prettyString);
    }

    void ImGuiStyleSerializer::deserialize(jsoncons::json json, ImGuiStyle &style)
    {
        style.FontSizeBase = json["FontSizeBase"].as<float>();
        style.FontScaleMain = json["FontScaleMain"].as<float>();
        style.FontScaleDpi = json["FontScaleDpi"].as<float>();

        style.Alpha = json["Alpha"].as<float>();
        style.DisabledAlpha = json["DisabledAlpha"].as<float>();

        style.WindowPadding = ImVec2(json["WindowPadding"][0].as<float>(), json["WindowPadding"][1].as<float>());
        style.WindowRounding = json["WindowRounding"].as<float>();
        style.WindowBorderSize = json["WindowBorderSize"].as<float>();
        style.WindowBorderHoverPadding = json["WindowBorderHoverPadding"].as<float>();
        style.WindowMinSize = ImVec2(json["WindowMinSize"][0].as<float>(), json["WindowMinSize"][1].as<float>());
        style.WindowTitleAlign = ImVec2(json["WindowTitleAlign"][0].as<float>(), json["WindowTitleAlign"][1].as<float>());
        style.WindowMenuButtonPosition = static_cast<ImGuiDir>(json["WindowMenuButtonPosition"].as<int>());

        style.ChildRounding = json["ChildRounding"].as<float>();
        style.ChildBorderSize = json["ChildBorderSize"].as<float>();

        style.PopupRounding = json["PopupRounding"].as<float>();
        style.PopupBorderSize = json["PopupBorderSize"].as<float>();

        style.FramePadding = ImVec2(json["FramePadding"][0].as<float>(), json["FramePadding"][1].as<float>());
        style.FrameRounding = json["FrameRounding"].as<float>();
        style.FrameBorderSize = json["FrameBorderSize"].as<float>();

        style.ItemSpacing = ImVec2(json["ItemSpacing"][0].as<float>(), json["ItemSpacing"][1].as<float>());
        style.ItemInnerSpacing = ImVec2(json["ItemInnerSpacing"][0].as<float>(), json["ItemInnerSpacing"][1].as<float>());
        style.CellPadding = ImVec2(json["CellPadding"][0].as<float>(), json["CellPadding"][1].as<float>());
        style.TouchExtraPadding = ImVec2(json["TouchExtraPadding"][0].as<float>(), json["TouchExtraPadding"][1].as<float>());

        style.IndentSpacing = json["IndentSpacing"].as<float>();
        style.ColumnsMinSpacing = json["ColumnsMinSpacing"].as<float>();

        style.ScrollbarSize = json["ScrollbarSize"].as<float>();
        style.ScrollbarRounding = json["ScrollbarRounding"].as<float>();
        style.ScrollbarPadding = json["ScrollbarPadding"].as<float>();

        style.GrabMinSize = json["GrabMinSize"].as<float>();
        style.GrabRounding = json["GrabRounding"].as<float>();

        style.LogSliderDeadzone = json["LogSliderDeadzone"].as<float>();
        style.ImageBorderSize = json["ImageBorderSize"].as<float>();

        style.TabRounding = json["TabRounding"].as<float>();
        style.TabBorderSize = json["TabBorderSize"].as<float>();
        style.TabMinWidthBase = json["TabMinWidthBase"].as<float>();
        style.TabMinWidthShrink = json["TabMinWidthShrink"].as<float>();
        style.TabCloseButtonMinWidthSelected = json["TabCloseButtonMinWidthSelected"].as<float>();
        style.TabCloseButtonMinWidthUnselected = json["TabCloseButtonMinWidthUnselected"].as<float>();

        style.TabBarBorderSize = json["TabBarBorderSize"].as<float>();
        style.TabBarOverlineSize = json["TabBarOverlineSize"].as<float>();

        style.TableAngledHeadersAngle = json["TableAngledHeadersAngle"].as<float>();
        style.TableAngledHeadersTextAlign = ImVec2(
            json["TableAngledHeadersTextAlign"][0].as<float>(),
            json["TableAngledHeadersTextAlign"][1].as<float>()
        );

        style.TreeLinesFlags = json["TreeLinesFlags"].as<int>();
        style.TreeLinesSize = json["TreeLinesSize"].as<float>();
        style.TreeLinesRounding = json["TreeLinesRounding"].as<float>();

        style.DragDropTargetRounding = json["DragDropTargetRounding"].as<float>();
        style.DragDropTargetBorderSize = json["DragDropTargetBorderSize"].as<float>();
        style.DragDropTargetPadding = json["DragDropTargetPadding"].as<float>();

        style.ColorMarkerSize = json["ColorMarkerSize"].as<float>();
        style.ColorButtonPosition = static_cast<ImGuiDir>(json["ColorButtonPosition"].as<int>());

        style.ButtonTextAlign = ImVec2(json["ButtonTextAlign"][0].as<float>(), json["ButtonTextAlign"][1].as<float>());
        style.SelectableTextAlign = ImVec2(json["SelectableTextAlign"][0].as<float>(), json["SelectableTextAlign"][1].as<float>());

        style.SeparatorTextBorderSize = json["SeparatorTextBorderSize"].as<float>();
        style.SeparatorTextAlign = ImVec2(json["SeparatorTextAlign"][0].as<float>(), json["SeparatorTextAlign"][1].as<float>());
        style.SeparatorTextPadding = ImVec2(json["SeparatorTextPadding"][0].as<float>(), json["SeparatorTextPadding"][1].as<float>());

        style.DisplayWindowPadding = ImVec2(json["DisplayWindowPadding"][0].as<float>(), json["DisplayWindowPadding"][1].as<float>());
        style.DisplaySafeAreaPadding = ImVec2(json["DisplaySafeAreaPadding"][0].as<float>(), json["DisplaySafeAreaPadding"][1].as<float>());

        style.DockingNodeHasCloseButton = json["DockingNodeHasCloseButton"].as<bool>();
        style.DockingSeparatorSize = json["DockingSeparatorSize"].as<float>();

        style.MouseCursorScale = json["MouseCursorScale"].as<float>();

        style.AntiAliasedLines = json["AntiAliasedLines"].as<bool>();
        style.AntiAliasedLinesUseTex = json["AntiAliasedLinesUseTex"].as<bool>();
        style.AntiAliasedFill = json["AntiAliasedFill"].as<bool>();

        style.CurveTessellationTol = json["CurveTessellationTol"].as<float>();
        style.CircleTessellationMaxError = json["CircleTessellationMaxError"].as<float>();

        style.HoverStationaryDelay = json["HoverStationaryDelay"].as<float>();
        style.HoverDelayShort = json["HoverDelayShort"].as<float>();
        style.HoverDelayNormal = json["HoverDelayNormal"].as<float>();

        style.HoverFlagsForTooltipMouse = json["HoverFlagsForTooltipMouse"].as<int>();
        style.HoverFlagsForTooltipNav = json["HoverFlagsForTooltipNav"].as<int>();

        style._MainScale = json["_MainScale"].as<float>();
        style._NextFrameFontSizeBase = json["_NextFrameFontSizeBase"].as<float>();

        const jc::json& colorsJson = json["Colors"];
        size_t maxIndex = std::min<size_t>(colorsJson.size(), ImGuiCol_COUNT);

        for (size_t i = 0; i < maxIndex; ++i)
        {
            const jc::json& c = colorsJson[i];
            style.Colors[i] = { c[0].as<float>(), c[1].as<float>(), c[2].as<float>(), c[3].as<float>() };
        }
    }

    void ImGuiStyleSerializer::deserialize(const std::filesystem::path& path, ImGuiStyle& style)
    {
        const auto string = grl::File::read(path.string());
        if(!string.has_value())
        {
            clogr::error("Could not load file: {}", path.string());
            return;
        }
        const jc::json json = jc::json::parse(string.value());
        deserialize(json, style);
    }
}
