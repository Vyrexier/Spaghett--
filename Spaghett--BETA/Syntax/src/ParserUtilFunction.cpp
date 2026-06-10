#include "../include/Spaghett/Parser.h"

namespace Spaghett
{
    void Parser::ViewAst() const
    {
        for (auto& stmt : m_statements)
            ViewNode(stmt.get(), 0);
    }

    void Parser::ViewNode(AstNode* node, int depth) const
    {
        if (!node) return;
        std::string pad(depth * 2, ' ');
        std::string arrow = depth > 0 ? "-> " : "";

        if (auto* n = dynamic_cast<AstLocalStatement*>(node))
        {
            std::cout << std::format("{}{}LocalStatement '{}'\n", pad, arrow, n->name);
            ViewNode(n->value.get(), depth + 1);
        }
        else if (auto* n = dynamic_cast<AstAssignment*>(node))
        {
            std::cout << std::format("{}{}Assignment '{}'\n", pad, arrow, n->name);
            ViewNode(n->value.get(), depth + 1);
        }
        else if (dynamic_cast<AstBreak*>(node))
            std::cout << std::format("{}{}Break\n", pad, arrow);

        else if (dynamic_cast<AstContinue*>(node))
            std::cout << std::format("{}{}Continue\n", pad, arrow);
        else if (auto* n = dynamic_cast<AstBinaryExpr*>(node))
        {
            std::cout << std::format("{}{}BinaryExpr '{}'\n", pad, arrow, n->op);
            ViewNode(n->left.get(), depth + 1);
            ViewNode(n->right.get(), depth + 1);
        }
        else if (auto* n = dynamic_cast<AstIndexAssignment*>(node))
        {
            std::cout << std::format("{}{}IndexAssignment\n", pad, arrow);
            ViewNode(n->table.get(), depth + 1);
            ViewNode(n->key.get(), depth + 1);
            ViewNode(n->value.get(), depth + 1);
        }
        else if (auto* n = dynamic_cast<AstIndexAssignment*>(node))
        {
            std::cout << std::format("{}{}IndexAssignment\n", pad, arrow);
            ViewNode(n->table.get(), depth + 1);
            ViewNode(n->key.get(), depth + 1);
            ViewNode(n->value.get(), depth + 1);
        }
        else if (auto* n = dynamic_cast<AstNumber*>(node))
            std::cout << std::format("{}{}Number '{}'\n", pad, arrow, n->value);
        else if (auto* n = dynamic_cast<AstString*>(node))
            std::cout << std::format("{}{}String '{}'\n", pad, arrow, n->value);
        else if (auto* n = dynamic_cast<AstIdentifier*>(node))
            std::cout << std::format("{}{}Identifier '{}'\n", pad, arrow, n->name);
        else if (auto* n = dynamic_cast<AstIfStatement*>(node))
        {
            std::cout << std::format("{}{}IfStatement\n", pad, arrow);
            std::cout << std::format("{}  Condition:\n", pad);
            ViewNode(n->condition.get(), depth + 2);
            std::cout << std::format("{}  Then:\n", pad);
            for (auto& s : n->then_block->statements)
                ViewNode(s.get(), depth + 2);
            for (auto& [cond, block] : n->else_ifs)
            {
                std::cout << std::format("{}  ElseIf:\n", pad);
                ViewNode(cond.get(), depth + 2);
                for (auto& s : block->statements)
                    ViewNode(s.get(), depth + 2);
            }
            if (n->else_block)
            {
                std::cout << std::format("{}  Else:\n", pad);
                for (auto& s : n->else_block->statements)
                    ViewNode(s.get(), depth + 2);
            }
        }
        else if (auto* n = dynamic_cast<AstWhileStatement*>(node))
        {
            std::cout << std::format("{}{}WhileStatement\n", pad, arrow);
            std::cout << std::format("{}  Condition:\n", pad);
            ViewNode(n->condition.get(), depth + 2);
            std::cout << std::format("{}  Body:\n", pad);
            for (auto& s : n->body->statements)
                ViewNode(s.get(), depth + 2);
        }
        else if (auto* n = dynamic_cast<AstFunctionCall*>(node))
        {
            std::cout << std::format("{}{}FunctionCall '{}'\n", pad, arrow, n->name);
            for (auto& arg : n->args)
                ViewNode(arg.get(), depth + 1);
        }
        else if (auto* n = dynamic_cast<AstFunctionDef*>(node))
        {
            std::cout << std::format("{}{}FunctionDef '{}' {}\n", pad, arrow, n->name, n->isLocal ? "(local)" : "(global)");
            for (auto& p : n->params)
                std::cout << std::format("{}  Param: {}\n", pad, p);
            for (auto& s : n->body->statements)
                ViewNode(s.get(), depth + 2);
        }
        else if (auto* n = dynamic_cast<AstReturnStatement*>(node))
        {
            std::cout << std::format("{}{}ReturnStatement\n", pad, arrow);
            for (auto& v : n->values)
                ViewNode(v.get(), depth + 1);
        }
        else if (auto* n = dynamic_cast<AstBool*>(node))
            std::cout << std::format("{}{}Bool '{}'\n", pad, arrow, n->value ? "true" : "false");
        else if (dynamic_cast<AstNull*>(node))
            std::cout << std::format("{}{}Null\n", pad, arrow);
        else if (auto* n = dynamic_cast<AstMultiAssign*>(node))
        {
            std::string names;
            for (size_t i = 0; i < n->names.size(); i++)
                names += (i > 0 ? ", " : "") + n->names[i];
            std::cout << std::format("{}{}MultiAssign '{}' {}\n", pad, arrow, names, n->isLocal ? "(local)" : "");
            ViewNode(n->value.get(), depth + 1);
        }
        else if (auto* n = dynamic_cast<AstUnaryExpr*>(node))
        {
            std::cout << std::format("{}{}UnaryExpr '{}'\n", pad, arrow, n->op);
            ViewNode(n->operand.get(), depth + 1);
        }
        else if (auto* n = dynamic_cast<AstTable*>(node))
        {
            std::cout << std::format("{}{}Table\n", pad, arrow);
            for (auto& f : n->fields)
            {
                if (!f.key.empty())
                    std::cout << std::format("{}  Key: {}\n", pad, f.key);
                else
                    std::cout << std::format("{}  ArrayItem\n", pad);
                ViewNode(f.value.get(), depth + 2);
            }
        }
        else if (auto* n = dynamic_cast<AstIndexExpr*>(node))
        {
            std::cout << std::format("{}{}IndexExpr\n", pad, arrow);
            ViewNode(n->table.get(), depth + 1);
            ViewNode(n->key.get(), depth + 1);
        }
        else if (auto* n = dynamic_cast<AstForNumeric*>(node))
        {
            std::cout << std::format("{}{}ForNumeric '{}'\n", pad, arrow, n->var);
            ViewNode(n->start.get(), depth + 1);
            ViewNode(n->limit.get(), depth + 1);
            if (n->step) ViewNode(n->step.get(), depth + 1);
            for (auto& s : n->body->statements)
                ViewNode(s.get(), depth + 2);
        }
        else if (auto* n = dynamic_cast<AstForIn*>(node))
        {
            std::cout << std::format("{}{}ForIn '{}, {}'\n", pad, arrow, n->key, n->value);
            ViewNode(n->table.get(), depth + 1);
            for (auto& s : n->body->statements)
                ViewNode(s.get(), depth + 2);
        }
        else if (auto* n = dynamic_cast<AstMethodCall*>(node))
        {
            std::cout << std::format("{}{}MethodCall '{}'\n", pad, arrow, n->method);
            ViewNode(n->object.get(), depth + 1);
            for (auto& arg : n->args)
                ViewNode(arg.get(), depth + 1);
                }
        else
            std::cout << std::format("{}{}Unknown\n", pad, arrow);
    }


    TokenWithData& Parser::Current()
    {
        if (m_position >= (int)m_tokens.size())
            return m_tokens.back();
        return m_tokens[m_position];
    }

    TokenWithData& Parser::Consume()
    {
        return m_tokens[m_position++];
    }

    bool Parser::Check(TokenType type)
    {
        return Current().type == type;
    }

    bool Parser::PeekCheck(TokenType type)
    {
        if (m_position + 1 >= m_tokens.size()) return false;
        return m_tokens[m_position + 1].type == type;
    }

    bool Parser::Expect(TokenType type)
    {
        if (Check(type)) { Consume(); return true; }
        m_errors.push_back("Expected " + TokenTypeToString(type) + " at line " + std::to_string(Current().location.begin.line) + " col " + std::to_string(Current().location.begin.column));
        return false;
    }
}