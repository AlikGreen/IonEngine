#include "stringParser.h"

namespace ion
{
    StringParser::StringParser(const std::string_view src)
        : m_src(src)
    {
    }

    void StringParser::skipWhitespace()
    {
        while (!done() && (peek() == ' ' || peek() == '\t' || peek() == '\n' || peek() == '\r'))
            ++m_pos;
    }


    void StringParser::skipToNextLine()
    {
        while (!done() && peek() != '\n') ++m_pos;
        if (!done()) ++m_pos;
    }

    std::string_view StringParser::consumeIdent()
    {
        skipWhitespace();
        const size_t start = m_pos;
        while (!done() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') || peek() == ':')
            ++m_pos;
        return m_src.substr(start, m_pos - start);
    }

    std::string_view StringParser::consumeIntLiteral()
    {
        skipWhitespace();

        const size_t start = m_pos;

        if (peek() == '+' || peek() == '-')
            ++m_pos;

        while (!done() && std::isdigit(static_cast<unsigned char>(peek())))
            ++m_pos;

        if (start == m_pos)
            return {};

        return m_src.substr(start, m_pos - start);
    }

    std::string_view StringParser::consumeStringLiteral()
    {
        if(!tryConsume('"')) return "";
        const auto str = readUntil("\"");
        if(done()) return "";
        tryConsume('"');
        return str;
    }

    bool StringParser::tryConsume(char c)
    {
        skipWhitespace();
        if (!done() && peek() == c) { ++m_pos; return true; }
        return false;
    }

    bool StringParser::tryConsume(const std::string_view c)
    {
        skipWhitespace();
        uint32_t size = 0;
        for(size_t i = 0; i < c.size(); i++)
        {
            if(done() || peek(i) != c[i]) return false;
            size++;
        }
        m_pos += size;
        return true;
    }

    std::string_view StringParser::restOfLine() const
    {
        auto end = m_src.find('\n', m_pos);
        if (end == std::string_view::npos) end = m_src.size();
        return m_src.substr(m_pos, end - m_pos);
    }

    std::string_view StringParser::consumeLine()
    {
        const auto line = restOfLine();
        m_pos += line.size();
        if (!done()) ++m_pos;
        return line;
    }

    std::string_view StringParser::readUntil(const std::string_view stopChars)
    {
        const size_t start = m_pos;
        while (!done() && stopChars.find(peek()) == std::string_view::npos)
            ++m_pos;
        return m_src.substr(start, m_pos - start);
    }

    std::string_view StringParser::subStr(const size_t first, const size_t last) const
    {
        return m_src.substr(first, last - first);
    }
}
