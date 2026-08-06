#pragma once

namespace ion
{
struct SourceLocation { uint32_t line = 0, column = 0; };
enum class DiagnosticSeverity { Warning, Error };

struct Diagnostic
{
    DiagnosticSeverity severity;
    std::string        message;
    SourceLocation      location;
};

class DiagnosticsSink
{
public:
    void report(DiagnosticSeverity severity, std::string message, SourceLocation loc = {});
    bool hasErrors() const { return m_hasErrors; }
    const std::vector<Diagnostic>& entries() const { return m_diagnostics; }

private:
    std::vector<Diagnostic> m_diagnostics;
    bool                    m_hasErrors = false;
};
}
