#pragma once
#include "glm/vec4.hpp"

namespace ion
{
struct AnsiStyle
{
    bool bold = false;
    bool dim = false;
    bool italic = false;
    bool underline = false;
    bool blink = false;
    bool inverse = false;
    bool hidden = false;
    bool strike = false;

    glm::vec4 fg = glm::vec4(1.0f);
    glm::vec4 bg = glm::vec4(1.0f);
};

struct AnsiTextSegment
{
    std::string text;
    AnsiStyle style;
};

class AnsiParser
{
public:
    static std::vector<AnsiTextSegment> parse(const std::string &text);
private:
    static bool isFinalByte(char c);
    static std::string readCsiSequence(const std::string &input, size_t &i);
    static std::vector<int> parseParams(const std::string &s);
    static void applySgr(const std::vector<int> &params, AnsiStyle &state);
};
}
