#include "ansiParser.h"

namespace ion
{
    std::vector<AnsiTextSegment> AnsiParser::parse(const std::string &text)
    {
        AnsiStyle state{};
        std::vector<AnsiTextSegment> result;

        constexpr char esc = '\x1B';

        std::string currentText;
        size_t i = 0;

        while(i < text.size())
        {
            if (text[i] == esc && i + 1 < text.size() && text[i + 1] == '[')
            {
                if (!currentText.empty())
                {
                    result.push_back({ currentText, state });
                    currentText.clear();
                }


                i += 2;
                std::string seq = readCsiSequence(text, i);
                const auto finalByte = seq.back();
                seq.pop_back();

                auto params = parseParams(seq);

                if (finalByte == 'm')
                {
                    applySgr(params, state);
                }
            }
            else
            {
                currentText += text[i];
                i++;
            }
        }

        if (!currentText.empty())
        {
            result.push_back({ currentText, state });
        }

        return result;
    }

    bool AnsiParser::isFinalByte(const char c)
    {
        const auto uc = static_cast<unsigned char>(c);
        return uc >= 0x40 && uc <= 0x7E;
    }

    std::string AnsiParser::readCsiSequence(const std::string& input, size_t& i)
    {
        std::string sequence;

        while (i < input.size())
        {
            const char c = input[i++];
            sequence += c;

            if (isFinalByte(c))
            {
                break;
            }
        }

        return sequence;
    }

    std::vector<int> AnsiParser::parseParams(const std::string& s)
    {
        std::vector<int> result;
        std::string current;

        for (char c : s)
        {
            if (c == ';')
            {
                if (!current.empty())
                {
                    result.push_back(std::stoi(current));
                    current.clear();
                }
            }
            else
            {
                current += c;
            }
        }

        if (!current.empty())
        {
            result.push_back(std::stoi(current));
        }

        return result;
    }

    glm::vec4 ansiColorToVec4(int code, bool bright)
    {
        static const glm::vec4 base[8] =
        {
            {0,0,0,1}, // black
            {1,0,0,1}, // red
            {0,1,0,1}, // green
            {1,1,0,1}, // yellow
            {0,0,1,1}, // blue
            {1,0,1,1}, // magenta
            {0,1,1,1}, // cyan
            {1,1,1,1}  // white
        };

        glm::vec4 c = base[code];
        if (bright)
        {
            c.r = std::min(c.r + 0.5f, 1.0f);
            c.g = std::min(c.g + 0.5f, 1.0f);
            c.b = std::min(c.b + 0.5f, 1.0f);
        }
        return c;
    }

    void AnsiParser::applySgr(const std::vector<int>& params, AnsiStyle& state)
    {
        if (params.empty())
        {
            state = {};
            return;
        }

        for (size_t i = 0; i < params.size(); ++i)
        {
            int p = params[i];

            switch (p)
            {
            case 0: state = {}; break;

            case 1: state.bold = true; state.dim = false; break;
            case 2: state.dim = true; state.bold = false; break;
            case 3: state.italic = true; break;
            case 4: state.underline = true; break;
            case 5: state.blink = true; break;
            case 7: state.inverse = true; break;
            case 8: state.hidden = true; break;
            case 9: state.strike = true; break;

            case 22: state.bold = false; state.dim = false; break;
            case 23: state.italic = false; break;
            case 24: state.underline = false; break;
            case 25: state.blink = false; break;
            case 27: state.inverse = false; break;
            case 28: state.hidden = false; break;
            case 29: state.strike = false; break;

            // foreground 30–37
            case 30: case 31: case 32: case 33:
            case 34: case 35: case 36: case 37:
                state.fg = ansiColorToVec4(p - 30, false);
                break;

            // bright foreground 90–97
            case 90: case 91: case 92: case 93:
            case 94: case 95: case 96: case 97:
                state.fg = ansiColorToVec4(p - 90, true);
                break;

            // background 40–47
            case 40: case 41: case 42: case 43:
            case 44: case 45: case 46: case 47:
                state.bg = ansiColorToVec4(p - 40, false);
                break;

            // bright background 100–107
            case 100: case 101: case 102: case 103:
            case 104: case 105: case 106: case 107:
                state.bg = ansiColorToVec4(p - 100, true);
                break;

            case 39: // reset fg
                state.fg = glm::vec4(1.0f);
                break;

            case 49: // reset bg
                state.bg = glm::vec4(0,0,0,1);
                break;

            // 256 / truecolor
            case 38: // fg extended
            case 48: // bg extended
            {
                bool isFg = (p == 38);

                if (i + 1 < params.size())
                {
                    int mode = params[++i];

                    if (mode == 5 && i + 1 < params.size())
                    {
                        int idx = params[++i];
                        glm::vec4 c;

                        if (idx < 16)
                        {
                            c = ansiColorToVec4(idx % 8, idx >= 8);
                        }
                        else if (idx < 232)
                        {
                            int v = idx - 16;
                            int r = (v / 36) % 6;
                            int g = (v / 6) % 6;
                            int b = v % 6;

                            c = glm::vec4(r / 5.0f, g / 5.0f, b / 5.0f, 1.0f);
                        }
                        else
                        {
                            float gray = (idx - 232) / 23.0f;
                            c = glm::vec4(gray, gray, gray, 1.0f);
                        }

                        if (isFg) state.fg = c;
                        else state.bg = c;
                    }
                    else if (mode == 2 && i + 3 < params.size())
                    {
                        float r = params[++i] / 255.0f;
                        float g = params[++i] / 255.0f;
                        float b = params[++i] / 255.0f;

                        glm::vec4 c(r, g, b, 1.0f);

                        if (isFg) state.fg = c;
                        else state.bg = c;
                    }
                }
                break;
            }

            default:
                break;
            }
        }
    }
}
