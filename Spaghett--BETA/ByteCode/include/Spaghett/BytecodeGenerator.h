#pragma once
#include "../../../Common/include/Spaghett/SpaghetState.h"
#include "../../../Common/include/Spaghett/Instruction.h"
#include "../../../Common/include/Spaghett/Chunk.h"
#include "../../../Common/include/Spaghett/Ast.h"
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>
#include <format>
#include <iostream>

namespace Spaghett
{

    struct LoopContext
    {
        int loopStart;
        int continueTarget;
        std::vector<int> breakJmps;
        std::vector<int> continueJmps;
    };

    class BytecodeGenerator
    {
    public:
        Chunk Generate(const std::vector<ptrNode>& statements, GlobalState* G = nullptr);

    private:
        uint8_t m_nextLocal = 0;
        GlobalState* m_G = nullptr;
        std::vector<LoopContext> m_loopStack;
        std::vector<std::pair<std::string, uint8_t>> m_outerSymbols;
        std::vector<UpvalueDesc>* m_outerUpvalues = nullptr;
        std::vector<UpvalueDesc>* m_parentOuterUpvalues = nullptr;
        std::vector<std::pair<std::string, uint8_t>> m_parentOuterSymbols;

        void GenNode(AstNode* node);
        void GenBlock(const std::vector<ptrNode>& statements);
        uint8_t GenExpr(AstNode* node);
        Chunk GenerateFunction(const std::vector<ptrNode>& statements);

        // register
        uint8_t NextRegister();
        uint8_t NextLocalRegister();
        void FreeRegister(uint8_t reg);
        bool IsLocalRegister(uint8_t reg);
        bool AllocatedRegisters[256] = {};

        // constants
        uint16_t AddConstant(TValue val);
        uint16_t AddString(const std::string& str);

        // locals
        uint8_t DeclareLocal(const std::string& name);
        int ResolveLocal(const std::string& name);
        std::vector<std::pair<std::string, uint8_t>> m_symbolTable;

        // emit
        void Emit(OperatorCode op, uint8_t A, uint16_t B, uint8_t C);
        void Emit(OperatorCode op, uint8_t A, uint16_t B);

        Chunk m_chunk;
    };
}