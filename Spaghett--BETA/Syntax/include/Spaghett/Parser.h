// Syntax/include/Spaghett/Parser.h
#pragma once
#include <vector>
#include <format>
#include <iostream>

#include "../../../Common/include/Spaghett/Ast.h"
#include "../../../Common/include/Spaghett/Token.h"

namespace Spaghett
{
    class Parser
    {
    public:
        Parser(std::vector<TokenWithData> tokens) : m_tokens(std::move(tokens)) {}
        ~Parser() = default;

        void Parse();
        const std::vector<ptrNode>& GetStatements() const { return m_statements; }
        const std::vector<std::string>& GetErrors() const { return m_errors; }

        bool Bad() const { return !m_errors.empty(); }

        void ViewAst() const;
        void ViewNode(AstNode* node, int depth) const;

    private:
        int m_position = 0;
        std::vector<TokenWithData> m_tokens;
        std::vector<ptrNode> m_statements;
        std::vector<std::string> m_errors;

        TokenWithData& Current();
        TokenWithData& Consume();
        bool Check(TokenType type);
        bool PeekCheck(TokenType type);
        bool Expect(TokenType type);
        bool IsEof() const { return m_tokens[m_position].type == TokenType::Eof; }

        ptrNode ParseStatement();
        ptrNode ParseLocalStatement();
        ptrNode ParseAssignment();
        ptrNode ParseExpression();
        ptrNode ParseComparison();
        ptrNode ParseAdditive();
        ptrNode ParseMultiplicative();
        ptrNode ParsePrimary();
        ptrNode ParseIfStatement();
        ptrNode ParseWhileStatement();
        ptrNode ParseFunctionCall(std::string name);
        ptrNode ParseFunctionDef(bool isLocal);
        ptrNode ParseReturnStatement();
        ptrNode ParseLogical();
        ptrNode ParseUnary();
        ptrNode ParseTable();
        ptrNode ParseForStatement();
        ptrNode ParseConcat();
        std::unique_ptr<AstBlock> ParseBlock();
    };
}