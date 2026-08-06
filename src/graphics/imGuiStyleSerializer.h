#pragma once
#include <imgui.h>

#include <jsoncons/json.hpp>


namespace ion
{
class ImGuiStyleSerializer
{
public:
    static jsoncons::json serialize(const ImGuiStyle& style);
    static void serialize(const std::filesystem::path &path, const ImGuiStyle &style);

    static void deserialize(jsoncons::json json, ImGuiStyle& style);
    static void deserialize(const std::filesystem::path& path, ImGuiStyle& style);
};
}
