#include "../include/Spaghett/SemanticAnalyzer.h"

namespace Spaghett
{
    void SemanticAnalyzer::Analyze(const std::vector<ptrNode>& statements)
    {
        PushScope(); // global

        for (auto& name : BuiltInRegistry::Get().GetAll())
            Declare(name);

        for (auto& stmt : statements)
            AnalyzeNode(stmt.get());
        PopScope();
    }

    void SemanticAnalyzer::PushScope() { m_scopes.push_back({}); }
    void SemanticAnalyzer::PopScope() { m_scopes.pop_back(); }

    void SemanticAnalyzer::Declare(const std::string& name)
    {
        m_scopes.back().insert(name);
    }

    bool SemanticAnalyzer::IsInScope(const std::string& name)
    {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it)
            if (it->count(name)) return true;
        return false;
    }

    void SemanticAnalyzer::AnalyzeNode(AstNode* node)
    {
        if (!node) return;

        if (auto* n = dynamic_cast<AstLocalStatement*>(node))
        {
            AnalyzeNode(n->value.get());
            Declare(n->name);
        }
        else if (auto* n = dynamic_cast<AstAssignment*>(node))
        {
            if (!IsInScope(n->name))
                m_errors.push_back("Error: '" + n->name + "' is not defined");
            AnalyzeNode(n->value.get());
        }
        else if (auto* n = dynamic_cast<AstIdentifier*>(node))
        {
            if (!IsInScope(n->name))
                m_errors.push_back("Error: '" + n->name + "' is not defined");
        }
        else if (auto* n = dynamic_cast<AstBinaryExpr*>(node))
        {
            AnalyzeNode(n->left.get());
            AnalyzeNode(n->right.get());
        }
        else if (auto* n = dynamic_cast<AstUnaryExpr*>(node))
        {
            AnalyzeNode(n->operand.get());
        }
        else if (auto* n = dynamic_cast<AstReturnStatement*>(node))
        {
            if (m_functionDepth == 0)
                m_errors.push_back("Error: 'return' outside function");
            for (auto& v : n->values)
                AnalyzeNode(v.get());
        }
        else if (auto* n = dynamic_cast<AstFunctionDef*>(node))
        {
            Declare(n->name);
            PushScope();
            m_functionDepth++;
            for (auto& p : n->params)
                Declare(p);
            for (auto& s : n->body->statements)
                AnalyzeNode(s.get());
            m_functionDepth--;
            PopScope();
        }
        else if (auto* n = dynamic_cast<AstFunctionCall*>(node))
        {
            if (!IsInScope(n->name))
                m_errors.push_back("Error: '" + n->name + "' is not defined");
            for (auto& arg : n->args)
                AnalyzeNode(arg.get());
        }
        else if (auto* n = dynamic_cast<AstIfStatement*>(node))
        {
            AnalyzeNode(n->condition.get());
            PushScope();
            for (auto& s : n->then_block->statements)
                AnalyzeNode(s.get());
            PopScope();
            for (auto& [cond, block] : n->else_ifs)
            {
                AnalyzeNode(cond.get());
                PushScope();
                for (auto& s : block->statements)
                    AnalyzeNode(s.get());
                PopScope();
            }
            if (n->else_block)
            {
                PushScope();
                for (auto& s : n->else_block->statements)
                    AnalyzeNode(s.get());
                PopScope();
            }
        }
        else if (auto* n = dynamic_cast<AstWhileStatement*>(node))
        {
            AnalyzeNode(n->condition.get());
            PushScope();
            for (auto& s : n->body->statements)
                AnalyzeNode(s.get());
            PopScope();
        }
        else if (auto* n = dynamic_cast<AstForNumeric*>(node))
        {
            AnalyzeNode(n->start.get());
            AnalyzeNode(n->limit.get());
            if (n->step) AnalyzeNode(n->step.get());
            PushScope();
            Declare(n->var);
            for (auto& s : n->body->statements)
                AnalyzeNode(s.get());
            PopScope();
        }
        else if (auto* n = dynamic_cast<AstForIn*>(node))
        {
            AnalyzeNode(n->table.get());
            PushScope();
            Declare(n->key);
            Declare(n->value);
            for (auto& s : n->body->statements)
                AnalyzeNode(s.get());
            PopScope();
        }
        else if (auto* n = dynamic_cast<AstTable*>(node))
        {
            for (auto& f : n->fields)
                AnalyzeNode(f.value.get());
        }
        else if (auto* n = dynamic_cast<AstIndexExpr*>(node))
        {
            AnalyzeNode(n->table.get());
            AnalyzeNode(n->key.get());
        }
    }
}