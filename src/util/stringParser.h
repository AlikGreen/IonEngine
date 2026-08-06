#pragma once

namespace ion
{
class StringParser
{
public:
    explicit StringParser(std::string_view src);

    [[nodiscard]] bool done() const { return m_pos >= m_src.size(); }
    [[nodiscard]] char peek(uint32_t n = 0) const { return m_src[m_pos+n]; }
    void skip(const size_t n = 1) { m_pos += n; }
    void reset() { m_pos = 0; }

    size_t pos() const { return m_pos; }
    void seek(const size_t pos) { m_pos = pos; }

    void skipWhitespace();
    void skipToNextLine();

    std::string_view consumeIdent();
    std::string_view consumeStringLiteral();
    std::string_view consumeIntLiteral();

    bool tryConsume(char c);
    bool tryConsume(std::string_view c);
    [[nodiscard]] std::string_view restOfLine() const;
    std::string_view consumeLine();
    std::string_view readUntil(std::string_view stopChars);

    // start inclusive, end exclusive
    [[nodiscard]] std::string_view subStr(size_t first, size_t last) const;
private:
    std::string_view m_src;
    size_t m_pos = 0;
};
}
