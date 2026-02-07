#pragma once
#include <string>
#include <utility>
#include <vector>

namespace ion
{
    struct Tag
    {
        Tag() = default;
        explicit Tag(std::string name) : name(std::move(name)) { };
        std::string name{};
        std::vector<std::string> tags{};
    };
}
