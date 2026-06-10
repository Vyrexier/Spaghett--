#pragma once
#include <memory>
#include <string>
#include <vector>

namespace Spaghett
{
    struct AstNode
    {
        virtual ~AstNode() = default;
    };

    using ptrNode = std::unique_ptr<AstNode>;

    struct AstBlock : AstNode
    {
        std::vector<ptrNode> statements;
    };

    struct AstBreak : AstNode {};
    struct AstContinue : AstNode {};

    struct AstMultiAssign : AstNode {
        std::vector<std::string> names;
        ptrNode value; // function call
        bool isLocal = false;
    };

    struct AstIfStatement : AstNode
    {
        ptrNode condition;
        std::unique_ptr<AstBlock> then_block;
        std::vector<std::pair<ptrNode, std::unique_ptr<AstBlock>>> else_ifs;
        std::unique_ptr<AstBlock> else_block;

        AstIfStatement(
            ptrNode cond,
            std::unique_ptr<AstBlock> then,
            std::vector<std::pair<ptrNode, std::unique_ptr<AstBlock>>> elseifs,
            std::unique_ptr<AstBlock> els)
            : condition(std::move(cond))
            , then_block(std::move(then))
            , else_ifs(std::move(elseifs))
            , else_block(std::move(els))
        {
        }
    };

    struct AstWhileStatement : AstNode
    {
        ptrNode condition;
        std::unique_ptr<AstBlock> body;
        AstWhileStatement(ptrNode cond, std::unique_ptr<AstBlock> body)
            : condition(std::move(cond)), body(std::move(body)) {
        }
    };

    struct AstFunctionDef : AstNode
    {
        std::string name;
        std::vector<std::string> params;
        std::unique_ptr<AstBlock> body;
        bool isLocal;
        AstFunctionDef(std::string name, std::vector<std::string> params, std::unique_ptr<AstBlock> body, bool isLocal)
            : name(std::move(name)), params(std::move(params)), body(std::move(body)), isLocal(isLocal) {
        }
    };

    struct AstFunctionCall : AstNode
    {
        std::string name;
        std::vector<ptrNode> args;
        AstFunctionCall(std::string name, std::vector<ptrNode> args)
            : name(std::move(name)), args(std::move(args)) {
        }
    };

    struct AstReturnStatement : AstNode {
        std::vector<ptrNode> values;
    };

    struct AstNumber : AstNode
    {
        double value;
        AstNumber(double v) : value(v) {}
    };

    struct AstString : AstNode
    {
        std::string value;
        AstString(std::string v) : value(std::move(v)) {}
    };

    struct AstBool : AstNode
    {
        bool value;
        AstBool(bool v) : value(v) {}
    };

    struct AstNull : AstNode {};

    struct AstUnaryExpr : AstNode
    {
        std::string op;
        ptrNode operand;
        AstUnaryExpr(std::string op, ptrNode operand)
            : op(std::move(op)), operand(std::move(operand)) {
        }
    };

    struct AstTable : AstNode
    {
        struct Field
        {
            std::string key;
            ptrNode value;
        };
        std::vector<Field> fields;
    };

    struct AstIdentifier : AstNode
    {
        std::string name;
        AstIdentifier(std::string n) : name(std::move(n)) {}
    };

    struct AstBinaryExpr : AstNode
    {
        ptrNode left;
        ptrNode right;
        std::string op;
        AstBinaryExpr(ptrNode l, std::string op, ptrNode r)
            : left(std::move(l)), op(std::move(op)), right(std::move(r)) {
        }
    };

    struct AstIndexExpr : AstNode
    {
        ptrNode table;
        ptrNode key;
        AstIndexExpr(ptrNode table, ptrNode key)
            : table(std::move(table)), key(std::move(key)) {
        }
    };

    struct AstIndexAssignment : AstNode
    {
        ptrNode table;
        ptrNode key;
        ptrNode value;
        AstIndexAssignment(ptrNode table, ptrNode key, ptrNode value)
            : table(std::move(table)), key(std::move(key)), value(std::move(value)) {
        }
    };

    struct AstForNumeric : AstNode
    {
        std::string var;
        ptrNode start, limit, step;
        std::unique_ptr<AstBlock> body;
        AstForNumeric(std::string var, ptrNode start, ptrNode limit, ptrNode step, std::unique_ptr<AstBlock> body)
            : var(std::move(var)), start(std::move(start)), limit(std::move(limit)), step(std::move(step)), body(std::move(body)) {
        }
    };

    struct AstForIn : AstNode
    {
        std::string key, value;
        ptrNode table;
        std::unique_ptr<AstBlock> body;
        AstForIn(std::string key, std::string value, ptrNode table, std::unique_ptr<AstBlock> body)
            : key(std::move(key)), value(std::move(value)), table(std::move(table)), body(std::move(body)) {
        }
    };

    struct AstMethodCall : AstNode
    {
        ptrNode object;        // t, os, string, os.io ...
        std::string method;
        std::vector<ptrNode> args;

        AstMethodCall(ptrNode obj, std::string method, std::vector<ptrNode> args)
            : object(std::move(obj)), method(std::move(method)), args(std::move(args)) {
        }
    };

    struct AstLocalStatement : AstNode
    {
        std::string name;
        ptrNode value;
        AstLocalStatement(std::string n, ptrNode v)
            : name(std::move(n)), value(std::move(v)) {
        }
    };

    struct AstAssignment : AstNode
    {
        std::string name;
        ptrNode value;
        AstAssignment(std::string n, ptrNode v)
            : name(std::move(n)), value(std::move(v)) {
        }
    };
}