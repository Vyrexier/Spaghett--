#include "../include/Spaghett/Parser.h"

namespace Spaghett
{
    void Parser::Parse()
    {
        while (!IsEof())
        {
            auto stmt = ParseStatement();
            if (stmt) m_statements.push_back(std::move(stmt));
        }
    }

    ptrNode Parser::ParseStatement()
    {
        if (IsEof()) return nullptr;

        if (Check(TokenType::ReservedBreak))
        {
            Consume();
            return std::make_unique<AstBreak>();
        }
        if (Check(TokenType::ReservedContinue))
        {
            Consume();
            return std::make_unique<AstContinue>();
        }

        if (Check(TokenType::ReservedLocal))
        {
            if (PeekCheck(TokenType::ReservedFunction))
            {
                Consume();
                return ParseFunctionDef(true);
            }
            return ParseLocalStatement();
        }

        if (Check(TokenType::ReservedReturn))
            return ParseReturnStatement();

        if (Check(TokenType::ReservedIf))
            return ParseIfStatement();

        if (PeekCheck(TokenType::LeftParen))
            return ParseExpression();

        if (Check(TokenType::Identifier))
        {
            if (PeekCheck(TokenType::Equal))
                return ParseAssignment();

            if (PeekCheck(TokenType::LeftParen))
                return ParseExpression();

            std::string name = Current().value;
            Consume();

            if (!Check(TokenType::Dot) && !Check(TokenType::LeftBracket))
            {
                m_position--;
                return ParseExpression();
            }

            ptrNode node = std::make_unique<AstIdentifier>(std::move(name));

            while (Check(TokenType::Dot) || Check(TokenType::LeftBracket))
            {
                if (Check(TokenType::Dot))
                {
                    Consume();
                    std::string key = Current().value;
                    Consume();
                    node = std::make_unique<AstIndexExpr>(
                        std::move(node),
                        std::make_unique<AstString>(std::move(key))
                    );
                }
                else
                {
                    Consume();
                    auto key = ParseExpression();
                    Expect(TokenType::RightBracket);
                    node = std::make_unique<AstIndexExpr>(std::move(node), std::move(key));
                }
            }

            if (Check(TokenType::Equal))
            {
                Consume();
                auto val = ParseExpression();
                auto* idx = dynamic_cast<AstIndexExpr*>(node.get());
                if (idx)
                    return std::make_unique<AstIndexAssignment>(
                        std::move(idx->table), std::move(idx->key), std::move(val));
            }
            else if (Check(TokenType::LeftParen))
            {
                Consume();
                std::vector<ptrNode> args;
                if (!Check(TokenType::RightParen))
                {
                    args.push_back(ParseExpression());
                    while (Check(TokenType::Comma)) { Consume(); args.push_back(ParseExpression()); }
                }
                Expect(TokenType::RightParen);
                return std::make_unique<AstMethodCall>(std::move(node), "", std::move(args));
            }

            return node;
        }

        if (Check(TokenType::ReservedWhile))
            return ParseWhileStatement();

        if (Check(TokenType::ReservedFunction))
            return ParseFunctionDef(false);

        if (Check(TokenType::ReservedFor))
            return ParseForStatement();

        m_errors.push_back("Unknown statement '" + Current().value +
            "' at line " + std::to_string(Current().location.begin.line));
        Consume();

        return nullptr;
    }

    ptrNode Parser::ParseLocalStatement()
    {
        Consume();
        std::string name = Current().value;
        Consume();

        if (Check(TokenType::Comma))
        {
            auto node = std::make_unique<AstMultiAssign>();
            node->names.push_back(name);
            while (Check(TokenType::Comma))
            {
                Consume();
                node->names.push_back(Current().value);
                Consume();
            }
            Expect(TokenType::Equal);
            node->value = ParseExpression();
            return node;
        }

        Expect(TokenType::Equal);
        auto value = ParseExpression();
        return std::make_unique<AstLocalStatement>(std::move(name), std::move(value));
    }

    ptrNode Parser::ParseAssignment()
    {
        std::string name = Current().value;
        Consume();

        if (Check(TokenType::Comma))
        {
            auto node = std::make_unique<AstMultiAssign>();
            node->names.push_back(name);
            while (Check(TokenType::Comma))
            {
                Consume();
                node->names.push_back(Current().value);
                Consume();
            }
            Expect(TokenType::Equal);
            node->value = ParseExpression();
            return node;
        }

        Expect(TokenType::Equal);
        auto value = ParseExpression();
        return std::make_unique<AstAssignment>(std::move(name), std::move(value));
    }

    ptrNode Parser::ParseReturnStatement()
    {
        Consume(); // return
        auto node = std::make_unique<AstReturnStatement>();
        node->values.push_back(ParseExpression());
        while (Check(TokenType::Comma))
        {
            Consume();
            node->values.push_back(ParseExpression());
        }
        return node;
    }

    ptrNode Parser::ParseExpression()
    {
        return ParseLogical();
    }

    ptrNode Parser::ParseComparison()
    {
        auto left = ParseConcat();

        while (Check(TokenType::Less) || Check(TokenType::LessEqual) ||
            Check(TokenType::Greater) || Check(TokenType::GreaterEqual) ||
            Check(TokenType::EqualEqual) || Check(TokenType::NotEqual))
        {
            std::string op = Current().value;
            Consume();
            auto right = ParseAdditive();
            left = std::make_unique<AstBinaryExpr>(std::move(left), op, std::move(right));
        }

        return left;
    }

    ptrNode Parser::ParseAdditive()
    {
        auto left = ParseMultiplicative();

        while (Check(TokenType::Plus) || Check(TokenType::Minus))
        {
            std::string op = Current().value;
            Consume();
            auto right = ParseMultiplicative();
            left = std::make_unique<AstBinaryExpr>(std::move(left), op, std::move(right));
        }

        return left;
    }

    ptrNode Parser::ParseMultiplicative()
    {
        auto left = ParsePrimary();

        while (Check(TokenType::Multiply) || Check(TokenType::Divide) || Check(TokenType::Modulo))
        {
            std::string op = Current().value;
            Consume();
            auto right = ParsePrimary();
            left = std::make_unique<AstBinaryExpr>(std::move(left), op, std::move(right));
        }

        return left;
    }

    ptrNode Parser::ParseFunctionCall(std::string name)
    {
        Expect(TokenType::LeftParen);
        std::vector<ptrNode> args;

        if (!Check(TokenType::RightParen))
        {
            args.push_back(ParseExpression());
            while (Check(TokenType::Comma)) { Consume(); args.push_back(ParseExpression()); }
        }
        Expect(TokenType::RightParen);

        ptrNode node = std::make_unique<AstFunctionCall>(std::move(name), std::move(args));

        // loop ต่อถ้ามี ()
        while (Check(TokenType::LeftParen))
        {
            Consume();
            std::vector<ptrNode> chainArgs;
            if (!Check(TokenType::RightParen))
            {
                chainArgs.push_back(ParseExpression());
                while (Check(TokenType::Comma)) { Consume(); chainArgs.push_back(ParseExpression()); }
            }
            Expect(TokenType::RightParen);
            node = std::make_unique<AstMethodCall>(std::move(node), "", std::move(chainArgs));
        }

        return node;
    }

    ptrNode Parser::ParseConcat()
    {
        auto left = ParseAdditive();
        while (Check(TokenType::Concat))
        {
            Consume();
            auto right = ParseAdditive();
            left = std::make_unique<AstBinaryExpr>(std::move(left), "..", std::move(right));
        }
        return left;
    }

    ptrNode Parser::ParseFunctionDef(bool isLocal)
    {
        Consume(); // func
        std::string name = Current().value;
        Consume();

        Expect(TokenType::LeftParen);
        std::vector<std::string> params;
        if (!Check(TokenType::RightParen))
        {
            params.push_back(Current().value);
            Consume();
            while (Check(TokenType::Comma))
            {
                Consume();
                params.push_back(Current().value);
                Consume();
            }
        }
        Expect(TokenType::RightParen);

        auto body = std::make_unique<AstBlock>();
        while (!Check(TokenType::ReservedEnd) && !IsEof())
        {
            auto stmt = ParseStatement();
            if (stmt) body->statements.push_back(std::move(stmt));
        }
        Expect(TokenType::ReservedEnd);

        return std::make_unique<AstFunctionDef>(std::move(name), std::move(params), std::move(body), isLocal);
    }

    ptrNode Parser::ParseLogical()
    {
        auto left = ParseUnary();
        while (Check(TokenType::ReservedAnd) || Check(TokenType::ReservedOr))
        {
            std::string op = Current().value;
            Consume();
            auto right = ParseUnary();
            left = std::make_unique<AstBinaryExpr>(std::move(left), op, std::move(right));
        }
        return left;
    }

    ptrNode Parser::ParseUnary()
    {
        if (Check(TokenType::ReservedNot))
        {
            Consume();
            return std::make_unique<AstUnaryExpr>("not", ParseUnary());
        }
        if (Check(TokenType::Minus))
        {
            Consume();
            return std::make_unique<AstUnaryExpr>("-", ParseUnary());
        }
        return ParseComparison();
    }

    ptrNode Parser::ParseTable()
    {
        Expect(TokenType::LeftBrace);
        auto table = std::make_unique<AstTable>();

        while (!Check(TokenType::RightBrace) && !IsEof())
        {
            AstTable::Field field;

            if (Check(TokenType::Identifier) && PeekCheck(TokenType::Equal))
            {
                field.key = Current().value;
                Consume(); // key
                Consume(); // =
            }

            field.value = ParseExpression();
            table->fields.push_back(std::move(field));

            if (Check(TokenType::Comma)) Consume();
        }

        Expect(TokenType::RightBrace);
        return table;
    }

    ptrNode Parser::ParsePrimary()
    {
        if (Check(TokenType::ReservedTrue)) { Consume(); return std::make_unique<AstBool>(true); }
        if (Check(TokenType::ReservedFalse)) { Consume(); return std::make_unique<AstBool>(false); }
        if (Check(TokenType::ReservedNull)) { Consume(); return std::make_unique<AstNull>(); }

        if (Check(TokenType::LeftBrace))
            return ParseTable();

        if (Check(TokenType::Number))
        {
            double val = std::stod(Current().value);
            Consume();
            return std::make_unique<AstNumber>(val);
        }

        if (Check(TokenType::String))
        {
            std::string val = Current().value;
            Consume();
            return std::make_unique<AstString>(std::move(val));
        }

        if (Check(TokenType::Identifier))
        {
            std::string name = Current().value;
            Consume();
            if (Check(TokenType::LeftParen))
                return ParseFunctionCall(name);

            ptrNode node = std::make_unique<AstIdentifier>(std::move(name));

            while (Check(TokenType::Dot) || Check(TokenType::LeftBracket) || Check(TokenType::LeftParen))
            {
                if (Check(TokenType::Dot))
                {
                    Consume();
                    std::string key = Current().value;
                    Consume();

                    if (Check(TokenType::LeftParen))
                    {
                        // os.read() → AstMethodCall
                        Consume(); // (
                        std::vector<ptrNode> args;
                        if (!Check(TokenType::RightParen))
                        {
                            args.push_back(ParseExpression());
                            while (Check(TokenType::Comma))
                            {
                                Consume();
                                args.push_back(ParseExpression());
                            }
                        }
                        Expect(TokenType::RightParen);
                        node = std::make_unique<AstMethodCall>(std::move(node), std::move(key), std::move(args));
                    }
                    else
                    {
                        node = std::make_unique<AstIndexExpr>(
                            std::move(node),
                            std::make_unique<AstString>(std::move(key))
                        );
                    }
                }
                else if (Check(TokenType::LeftBracket))
                {
                    Consume();
                    auto key = ParseExpression();
                    Expect(TokenType::RightBracket);
                    node = std::make_unique<AstIndexExpr>(std::move(node), std::move(key));
                }
                else if (Check(TokenType::LeftParen))
                {
                    // expr() — call on expression
                    Consume();
                    std::vector<ptrNode> args;
                    if (!Check(TokenType::RightParen))
                    {
                        args.push_back(ParseExpression());
                        while (Check(TokenType::Comma))
                        {
                            Consume();
                            args.push_back(ParseExpression());
                        }
                    }
                    Expect(TokenType::RightParen);
                    node = std::make_unique<AstMethodCall>(std::move(node), "", std::move(args));
                }
            }
            return node;
        }

        if (Check(TokenType::ReservedFunction))
        {
            Consume(); // function
            Expect(TokenType::LeftParen);
            std::vector<std::string> params;
            if (!Check(TokenType::RightParen))
            {
                params.push_back(Current().value);
                Consume();
                while (Check(TokenType::Comma))
                {
                    Consume();
                    params.push_back(Current().value);
                    Consume();
                }
            }
            Expect(TokenType::RightParen);

            auto body = std::make_unique<AstBlock>();
            while (!Check(TokenType::ReservedEnd) && !IsEof())
            {
                auto stmt = ParseStatement();
                if (stmt) body->statements.push_back(std::move(stmt));
            }
            Expect(TokenType::ReservedEnd);

            return std::make_unique<AstFunctionDef>("", std::move(params), std::move(body), true);
        }

        if (Check(TokenType::LeftParen))
        {
            Consume();
            auto expr = ParseExpression();
            Expect(TokenType::RightParen);
            return expr;
        }

        m_errors.push_back("Unexpected token '" + Current().value +
            "' at line " + std::to_string(Current().location.begin.line) +
            " col " + std::to_string(Current().location.begin.column));
        Consume();
        return nullptr;
    }

    ptrNode Parser::ParseForStatement()
    {
        Consume(); // for
        std::string var = Current().value;
        Consume();

        if (Check(TokenType::Equal)) // numeric
        {
            Consume();
            auto start = ParseExpression();
            Expect(TokenType::Comma);
            auto limit = ParseExpression();
            ptrNode step;
            if (Check(TokenType::Comma))
            {
                Consume();
                step = ParseExpression();
            }
            Expect(TokenType::ReservedDo);
            auto body = std::make_unique<AstBlock>();
            while (!Check(TokenType::ReservedEnd) && !IsEof())
            {
                auto stmt = ParseStatement();
                if (stmt) body->statements.push_back(std::move(stmt));
            }
            Expect(TokenType::ReservedEnd);
            return std::make_unique<AstForNumeric>(std::move(var), std::move(start), std::move(limit), std::move(step), std::move(body));
        }
        else // for in
        {
            Expect(TokenType::Comma);
            std::string value = Current().value;
            Consume();
            Expect(TokenType::ReservedIn);
            auto table = ParseExpression();
            Expect(TokenType::ReservedDo);
            auto body = std::make_unique<AstBlock>();
            while (!Check(TokenType::ReservedEnd) && !IsEof())
            {
                auto stmt = ParseStatement();
                if (stmt) body->statements.push_back(std::move(stmt));
            }
            Expect(TokenType::ReservedEnd);
            return std::make_unique<AstForIn>(std::move(var), std::move(value), std::move(table), std::move(body));
        }
    }

    std::unique_ptr<AstBlock> Parser::ParseBlock()
    {
        Expect(TokenType::LeftBrace);
        auto block = std::make_unique<AstBlock>();
        while (!Check(TokenType::RightBrace) && !IsEof())
        {
            auto stmt = ParseStatement();
            if (stmt) block->statements.push_back(std::move(stmt));
        }
        Expect(TokenType::RightBrace);
        return block;
    }

    ptrNode Parser::ParseIfStatement()
    {
        Consume(); // if
        auto condition = ParseExpression();
        Expect(TokenType::ReservedThen); // then

        auto then_block = std::make_unique<AstBlock>();
        while (!Check(TokenType::ReservedEnd) &&
            !Check(TokenType::ReservedElse) &&
            !Check(TokenType::ReservedElseIf) &&
            !IsEof())
        {
            auto stmt = ParseStatement();
            if (stmt) then_block->statements.push_back(std::move(stmt));
        }

        std::vector<std::pair<ptrNode, std::unique_ptr<AstBlock>>> else_ifs;
        std::unique_ptr<AstBlock> else_block;

        while (Check(TokenType::ReservedElse) || Check(TokenType::ReservedElseIf))
        {
            if (Check(TokenType::ReservedElseIf) ||
                (Check(TokenType::ReservedElse) && PeekCheck(TokenType::ReservedIf)))
            {
                Consume(); // elseif หรือ else
                if (Check(TokenType::ReservedIf)) Consume(); // if (กรณี else if)

                auto elif_cond = ParseExpression();
                Expect(TokenType::ReservedThen);
                auto elif_block = std::make_unique<AstBlock>();
                while (!Check(TokenType::ReservedEnd) &&
                    !Check(TokenType::ReservedElse) &&
                    !Check(TokenType::ReservedElseIf) &&
                    !IsEof())
                {
                    auto stmt = ParseStatement();
                    if (stmt) elif_block->statements.push_back(std::move(stmt));
                }
                else_ifs.push_back({ std::move(elif_cond), std::move(elif_block) });
            }
            else
            {
                Consume(); // else
                else_block = std::make_unique<AstBlock>();
                while (!Check(TokenType::ReservedEnd) && !IsEof())
                {
                    auto stmt = ParseStatement();
                    if (stmt) else_block->statements.push_back(std::move(stmt));
                }
                break;
            }
        }

        Expect(TokenType::ReservedEnd);
        return std::make_unique<AstIfStatement>(
            std::move(condition),
            std::move(then_block),
            std::move(else_ifs),
            std::move(else_block)
        );
    }

    ptrNode Parser::ParseWhileStatement()
    {
        Consume(); // while
        auto condition = ParseExpression();
        Expect(TokenType::ReservedDo);

        auto body = std::make_unique<AstBlock>();
        while (!Check(TokenType::ReservedEnd) && !IsEof())
        {
            auto stmt = ParseStatement();
            if (stmt) body->statements.push_back(std::move(stmt));
        }

        Expect(TokenType::ReservedEnd);
        return std::make_unique<AstWhileStatement>(std::move(condition), std::move(body));
    }
}