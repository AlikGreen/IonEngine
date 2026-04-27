using System.Collections.Immutable;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.Diagnostics;
using DiagnosticDescriptor = Microsoft.CodeAnalysis.DiagnosticDescriptor;
using DiagnosticSeverity = Microsoft.CodeAnalysis.DiagnosticSeverity;
using LanguageNames = Microsoft.CodeAnalysis.LanguageNames;

namespace IonEngine;

[DiagnosticAnalyzer(LanguageNames.CSharp)]
public class RefReturnAnalyzer : DiagnosticAnalyzer
{
    public static readonly DiagnosticDescriptor Rule = new(
        id: "ION001",
        title: "Get<T>() must be assigned to a ref local",
        messageFormat: "Use 'ref var {0} = ref entity.Get<T>()' to avoid copying the component",
        category: "Usage",
        defaultSeverity: DiagnosticSeverity.Error,
        isEnabledByDefault: true
    );

    public override ImmutableArray<DiagnosticDescriptor> SupportedDiagnostics => ImmutableArray.Create(Rule);

    public override void Initialize(AnalysisContext context)
    {
        context.EnableConcurrentExecution();
        context.ConfigureGeneratedCodeAnalysis(GeneratedCodeAnalysisFlags.None);
        context.RegisterSyntaxNodeAction(Analyze, SyntaxKind.InvocationExpression);
    }

    private void Analyze(SyntaxNodeAnalysisContext context)
    {
        var invocation = (InvocationExpressionSyntax)context.Node;

        // Check it's a call to .Get<T>()
        if (invocation.Expression is not MemberAccessExpressionSyntax member
            || member.Name.Identifier.Text != "Get")
            return;

        var symbol = context.SemanticModel.GetSymbolInfo(invocation).Symbol as IMethodSymbol;
        if (symbol is null || !symbol.ReturnsByRef)
            return;

        // Walk up — is the parent a ref assignment?
        var parent = invocation.Parent;

        bool isRefAssignment = parent is AssignmentExpressionSyntax assign
                               && assign.Parent is RefExpressionSyntax;

        bool isRefDeclaration = parent is RefExpressionSyntax refExpr
                                && refExpr.Parent is EqualsValueClauseSyntax equals
                                && equals.Parent is VariableDeclaratorSyntax decl
                                && decl.Parent is VariableDeclarationSyntax varDecl
                                && varDecl.Type is RefTypeSyntax;
        
        bool isRefReturn = parent is RefExpressionSyntax refReturn
                           && refReturn.Parent is ArrowExpressionClauseSyntax;

        if (!isRefAssignment && !isRefDeclaration &&!isRefReturn)
        {
            var varName = "transform"; // fallback hint
            context.ReportDiagnostic(Diagnostic.Create(Rule, invocation.GetLocation(), varName));
        }
    }
}