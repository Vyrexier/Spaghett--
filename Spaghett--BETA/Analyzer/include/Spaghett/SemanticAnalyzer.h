#pragma once
#pragma once
#include "../../../Common/include/Spaghett/Ast.h"
#include "BuiltInRegistry.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Spaghett
{
    class SemanticAnalyzer
    {
    public:
        void Analyze(const std::vector<ptrNode>& statements);
        const std::vector<std::string>& GetErrors() const { return m_errors; }

    private:
        void AnalyzeNode(AstNode* node);

        void PushScope();
        void PopScope();
        void Declare(const std::string& name);
        bool IsInScope(const std::string& name);

        std::vector<std::unordered_set<std::string>> m_scopes;
        std::vector<std::string> m_errors;
        int m_functionDepth = 0;
    };
}