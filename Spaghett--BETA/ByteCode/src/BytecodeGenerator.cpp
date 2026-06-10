#include "../include/Spaghett/BytecodeGenerator.h"
#include "../../Common/include/Spaghett/Instruction.h"

#include <iostream>
#include <format>

namespace Spaghett
{
    Chunk BytecodeGenerator::Generate(const std::vector<ptrNode>& statements, GlobalState* G)
    {
        m_chunk = Chunk{};
        m_G = G;
        memset(AllocatedRegisters, false, sizeof(AllocatedRegisters));
        m_symbolTable.clear();
        m_nextLocal = 0;
        GenBlock(statements);
        return std::move(m_chunk);
    }

    void BytecodeGenerator::GenBlock(const std::vector<ptrNode>& statements)
    {
        std::cout << "GenBlock: " << statements.size() << " statements\n";
        for (auto& stmt : statements)
            GenNode(stmt.get());
    }
    
    void BytecodeGenerator::GenNode(AstNode* node)
    {
        if (auto* n = dynamic_cast<AstLocalStatement*>(node))
        {
            uint8_t rVal = GenExpr(n->value.get());
            uint8_t rDst = NextLocalRegister();
            if (rVal != rDst)
                Emit(OperatorCode::MOVE, rDst, rVal);
            FreeRegister(rVal);
            m_symbolTable.push_back({ n->name, rDst });
        }
        else if (dynamic_cast<AstBreak*>(node))
        {
            int j = (int)m_chunk.code.size();
            Emit(OperatorCode::JMP, 0, 0);
            m_loopStack.back().breakJmps.push_back(j);
        }
        else if (auto* n = dynamic_cast<AstForIn*>(node))
        {
            uint8_t rTable = GenExpr(n->table.get());
            uint8_t rIdx = NextLocalRegister();
            uint8_t rDone = NextLocalRegister(); // flag
            uint8_t rKey = NextLocalRegister();
            uint8_t rVal = NextLocalRegister();

            Emit(OperatorCode::FORPREP, rTable, rIdx);

            int loopStart = (int)m_chunk.code.size();
            m_loopStack.push_back({ loopStart, {} });

            Emit(OperatorCode::FORNEXT, rTable, rIdx, rKey); // sets rDone,rKey,rVal
            int jmpDone = (int)m_chunk.code.size();
            Emit(OperatorCode::JMPFALSE, rDone, 0); // patch ทีหลัง

            m_symbolTable.push_back({ n->key,   rKey });
            m_symbolTable.push_back({ n->value, rVal });

            for (auto& s : n->body->statements)
                GenNode(s.get());

            int incrementPos = (int)m_chunk.code.size();
            for (int j : m_loopStack.back().continueJmps)
                m_chunk.code[j].B = (uint16_t)(incrementPos - j - 1);

            int jmpBack = (int)m_chunk.code.size();
            Emit(OperatorCode::JMP, 0, 0);
            m_chunk.code[jmpBack].B = (uint16_t)(loopStart - jmpBack - 1);

            m_chunk.code[jmpDone].B = (uint16_t)(m_chunk.code.size() - jmpDone - 1);

            for (int j : m_loopStack.back().breakJmps)
                m_chunk.code[j].B = (uint16_t)(m_chunk.code.size() - j - 1);
            m_loopStack.pop_back();

            m_symbolTable.pop_back();
            m_symbolTable.pop_back();
            FreeRegister(rVal);
            FreeRegister(rKey);
            FreeRegister(rDone);
            FreeRegister(rIdx);
        }
        else if (dynamic_cast<AstContinue*>(node))
        {
            int j = (int)m_chunk.code.size();
            Emit(OperatorCode::JMP, 0, 0);
            m_loopStack.back().continueJmps.push_back(j);
        }
        else if (auto* n = dynamic_cast<AstMultiAssign*>(node))
        {
            uint8_t rCall = GenExpr(n->value.get());
            for (size_t i = 0; i < n->names.size(); i++)
            {
                if (n->isLocal)
                {
                    uint8_t rDst = NextLocalRegister();
                    Emit(OperatorCode::MOVE, rDst, rCall + (uint8_t)i);
                    m_symbolTable.push_back({ n->names[i], rDst });
                }
                else
                {
                    uint8_t rTmp = NextRegister();
                    Emit(OperatorCode::MOVE, rTmp, rCall + (uint8_t)i);
                    int local = ResolveLocal(n->names[i]);
                    if (local >= 0)
                        Emit(OperatorCode::MOVE, (uint8_t)local, rTmp);
                    else
                    {
                        uint16_t sidx = AddString(n->names[i]);
                        TValue tv; tv.type = Type::STRING; tv.str = sidx;
                        uint16_t idx = AddConstant(tv);
                        Emit(OperatorCode::SETGLOBAL, rTmp, idx);
                    }
                    FreeRegister(rTmp);
                }
            }
            FreeRegister(rCall);
        }
        else if (auto* n = dynamic_cast<AstIndexAssignment*>(node))
        {
            uint8_t rTable = GenExpr(n->table.get());
            uint8_t rVal = GenExpr(n->value.get());

            if (auto* s = dynamic_cast<AstString*>(n->key.get()))
            {
                uint16_t sidx = AddString(s->value);
                TValue tv; tv.type = Type::STRING; tv.str = sidx;
                uint16_t kidx = AddConstant(tv);
                Emit(OperatorCode::SETFIELD, rTable, kidx, rVal);
            }

            FreeRegister(rVal);
            FreeRegister(rTable);
        }
        else if (auto* n = dynamic_cast<AstIfStatement*>(node))
        {
            std::vector<int> jmpEnds;

            uint8_t rCond = GenExpr(n->condition.get());
            int jmpFalse = (int)m_chunk.code.size();
            Emit(OperatorCode::JMPFALSE, rCond, 0);
            FreeRegister(rCond);

            for (auto& s : n->then_block->statements)
                GenNode(s.get());

            jmpEnds.push_back((int)m_chunk.code.size());
            Emit(OperatorCode::JMP, 0, 0);
            m_chunk.code[jmpFalse].B = (uint16_t)(m_chunk.code.size() - jmpFalse - 1);

            for (auto& ei : n->else_ifs)
            {
                uint8_t rC = GenExpr(ei.first.get());
                int jmpF = (int)m_chunk.code.size();
                Emit(OperatorCode::JMPFALSE, rC, 0);
                FreeRegister(rC);

                for (auto& s : ei.second->statements)
                    GenNode(s.get());

                jmpEnds.push_back((int)m_chunk.code.size());
                Emit(OperatorCode::JMP, 0, 0);
                m_chunk.code[jmpF].B = (uint16_t)(m_chunk.code.size() - jmpF - 1);
            }

            if (n->else_block)
                for (auto& s : n->else_block->statements)
                    GenNode(s.get());

            for (int j : jmpEnds)
                m_chunk.code[j].B = (uint16_t)(m_chunk.code.size() - j - 1);
        }
        else if (auto* n = dynamic_cast<AstAssignment*>(node))
        {
            uint8_t rVal = GenExpr(n->value.get());
            int local = ResolveLocal(n->name);
            if (local >= 0)
                Emit(OperatorCode::MOVE, (uint8_t)local, rVal);
            else
            {
                // เช็ค upvalue
                for (size_t i = 0; i < m_chunk.upvalues.size(); i++)
                {
                    if (m_chunk.upvalues[i].name == n->name)
                    {
                        Emit(OperatorCode::SETUPVAL, rVal, (uint16_t)i);
                        FreeRegister(rVal);
                        return;
                    }
                }
                // global
                uint16_t sidx = AddString(n->name);
                TValue tv; tv.type = Type::STRING; tv.str = sidx;
                uint16_t idx = AddConstant(tv);
                Emit(OperatorCode::SETGLOBAL, rVal, idx);
            }
            FreeRegister(rVal);
        }
        else if (auto* n = dynamic_cast<AstReturnStatement*>(node))
        {
            uint8_t rFirst = 0;
            for (size_t i = 0; i < n->values.size(); i++)
            {
                uint8_t r = GenExpr(n->values[i].get());
                if (i == 0) rFirst = r;
                else Emit(OperatorCode::MOVE, rFirst + (uint8_t)i, r);
            }
            Emit(OperatorCode::RETURN, rFirst, (uint16_t)n->values.size());
            FreeRegister(rFirst);
}
        else if (auto* n = dynamic_cast<AstFunctionDef*>(node))
        {
            BytecodeGenerator subGen;
            for (size_t i = 0; i < n->params.size(); i++)
                subGen.m_symbolTable.push_back({ n->params[i], (uint8_t)i });
            subGen.m_nextLocal = (uint8_t)n->params.size();

            subGen.m_outerSymbols = m_symbolTable;
            subGen.m_outerUpvalues = &m_chunk.upvalues;
            subGen.m_parentOuterUpvalues = m_outerUpvalues;
            subGen.m_parentOuterSymbols = m_outerSymbols;

            Chunk* funcChunk = new Chunk(subGen.GenerateFunction(n->body->statements));
            uint16_t cidx = (uint16_t)m_chunk.subchunks.size();
            m_chunk.subchunks.push_back(funcChunk);

            uint8_t reg = NextRegister();
            Emit(OperatorCode::CLOSURE, reg, cidx);

            // ถ้าอยู่ใน function อื่น บังคับเป็น local เสมอ
            bool forceLocal = !m_outerSymbols.empty() || m_outerUpvalues != nullptr;

            if (n->isLocal || forceLocal)
            {
                uint8_t rDst = NextLocalRegister();
                if (reg != rDst) Emit(OperatorCode::MOVE, rDst, reg);
                FreeRegister(reg);
                m_symbolTable.push_back({ n->name, rDst });
            }
            else
            {
                uint16_t sidx = AddString(n->name);
                TValue tv; tv.type = Type::STRING; tv.str = sidx;
                uint16_t idx = AddConstant(tv);
                Emit(OperatorCode::SETGLOBAL, reg, idx);
                FreeRegister(reg);
            }
        }
        else if (auto* n = dynamic_cast<AstWhileStatement*>(node))
        {
            int loopStart = (int)m_chunk.code.size();

            uint8_t rCond = GenExpr(n->condition.get());
            int jmpFalse = (int)m_chunk.code.size();
            Emit(OperatorCode::JMPFALSE, rCond, 0);
            FreeRegister(rCond);

            m_loopStack.push_back({ loopStart, {} });

            for (auto& s : n->body->statements)
                GenNode(s.get());

            int jmpBack = (int)m_chunk.code.size();
            Emit(OperatorCode::JMP, 0, 0);
            m_chunk.code[jmpBack].B = (uint16_t)(loopStart - jmpBack - 1);
            m_chunk.code[jmpFalse].B = (uint16_t)(m_chunk.code.size() - jmpFalse - 1);

            for (int j : m_loopStack.back().continueJmps)
                m_chunk.code[j].B = (uint16_t)(loopStart - j - 1);
            m_loopStack.pop_back();
        }
        else if (auto* n = dynamic_cast<AstForNumeric*>(node))
        {
            uint8_t rVar = NextLocalRegister();
            uint8_t rLimit = NextLocalRegister();
            uint8_t rStep = NextLocalRegister();

            uint8_t rStart = GenExpr(n->start.get());
            Emit(OperatorCode::MOVE, rVar, rStart);
            FreeRegister(rStart);

            uint8_t rLim = GenExpr(n->limit.get());
            Emit(OperatorCode::MOVE, rLimit, rLim);
            FreeRegister(rLim);

            if (n->step)
            {
                uint8_t rSt = GenExpr(n->step.get());
                Emit(OperatorCode::MOVE, rStep, rSt);
                FreeRegister(rSt);
            }
            else
            {
                uint16_t kidx = AddConstant(TValue::Number(1));
                Emit(OperatorCode::LOADK, rStep, kidx);
            }

            m_symbolTable.push_back({ n->var, rVar });

            int loopStart = (int)m_chunk.code.size();
            m_loopStack.push_back({ loopStart, {} });

            uint8_t rCond = NextRegister();
            Emit(OperatorCode::LE, rCond, rVar, rLimit);
            int jmpFalse = (int)m_chunk.code.size();
            Emit(OperatorCode::JMPFALSE, rCond, 0);
            FreeRegister(rCond);

            for (auto& s : n->body->statements)
                GenNode(s.get());

            // patch continue
            int incrementPos = (int)m_chunk.code.size();
            for (int j : m_loopStack.back().continueJmps)
                m_chunk.code[j].B = (uint16_t)(incrementPos - j - 1);

            // i = i + step
            uint8_t rTmp = NextRegister();
            Emit(OperatorCode::ADD, rTmp, rVar, rStep);
            Emit(OperatorCode::MOVE, rVar, rTmp);
            FreeRegister(rTmp);

            int jmpBack = (int)m_chunk.code.size();
            Emit(OperatorCode::JMP, 0, 0);
            m_chunk.code[jmpBack].B = (uint16_t)(loopStart - jmpBack - 1);
            m_chunk.code[jmpFalse].B = (uint16_t)(m_chunk.code.size() - jmpFalse - 1);

            for (int j : m_loopStack.back().breakJmps) // patch break
                m_chunk.code[j].B = (uint16_t)(m_chunk.code.size() - j - 1);
            m_loopStack.pop_back();
            // cleanup locals
            m_symbolTable.pop_back();
            FreeRegister(rStep);
            FreeRegister(rLimit);
        }
        else if (auto* n = dynamic_cast<AstMethodCall*>(node))
        {
            uint8_t r = GenExpr(node);
            FreeRegister(r);
        }
        else if (auto* n = dynamic_cast<AstFunctionCall*>(node))
        {
            uint8_t r = GenExpr(node);
            FreeRegister(r);
        }
    }

    uint8_t BytecodeGenerator::GenExpr(AstNode* node)
    {
        if (auto* n = dynamic_cast<AstNumber*>(node))
        {
            uint8_t reg = NextRegister();
            uint16_t idx = AddConstant(TValue::Number(n->value));
            Emit(OperatorCode::LOADK, reg, idx);
            return reg;
        }
        else if (auto* n = dynamic_cast<AstBool*>(node))
        {
            uint8_t reg = NextRegister();
            uint16_t idx = AddConstant(TValue::Boolean(n->value));
            Emit(OperatorCode::LOADK, reg, idx);
            return reg;
        }
        else if (auto* n = dynamic_cast<AstTable*>(node))
        {
            uint8_t rA = NextRegister();
            Emit(OperatorCode::NEWTABLE, rA, 0);
            for (auto& f : n->fields)
            {
                uint8_t rVal = GenExpr(f.value.get());
                std::string key = f.key.empty() ? std::to_string(&f - &n->fields[0]) : f.key;
                uint16_t sidx = AddString(key);
                TValue tv; tv.type = Type::STRING; tv.str = sidx;
                uint16_t kidx = AddConstant(tv);
                Emit(OperatorCode::SETFIELD, rA, kidx, rVal);
                // A=table, B=key idx (uint16_t), C=val register
                FreeRegister(rVal);
            }
            return rA;
        }
        else if (auto* n = dynamic_cast<AstFunctionDef*>(node))
        {
            BytecodeGenerator subGen;
            for (size_t i = 0; i < n->params.size(); i++)
                subGen.m_symbolTable.push_back({ n->params[i], (uint8_t)i });
            subGen.m_nextLocal = (uint8_t)n->params.size();

            subGen.m_outerSymbols = m_symbolTable;
            subGen.m_outerUpvalues = &m_chunk.upvalues;
            subGen.m_parentOuterUpvalues = m_outerUpvalues;
            subGen.m_parentOuterSymbols = m_outerSymbols;

            Chunk* funcChunk = new Chunk(subGen.GenerateFunction(n->body->statements));
            uint16_t cidx = (uint16_t)m_chunk.subchunks.size();
            m_chunk.subchunks.push_back(funcChunk);

            uint8_t reg = NextRegister();
            Emit(OperatorCode::CLOSURE, reg, cidx);
            return reg;
        }
        else if (auto* n = dynamic_cast<AstIndexExpr*>(node))
        {
            uint8_t rTable = GenExpr(n->table.get());
            uint8_t rA = NextRegister();

            if (auto* s = dynamic_cast<AstString*>(n->key.get()))
            {
                uint16_t sidx = AddString(s->value);
                TValue tv; tv.type = Type::STRING; tv.str = sidx;
                uint16_t kidx = AddConstant(tv);
                Emit(OperatorCode::GETFIELD, rA, kidx, rTable);
            }
            else if (auto* num = dynamic_cast<AstNumber*>(n->key.get()))
            {
                uint16_t kidx = AddConstant(TValue::Number(num->value));
                Emit(OperatorCode::GETFIELD, rA, kidx, rTable);
            }

            return rA;
        }
        else if (auto* n = dynamic_cast<AstIndexExpr*>(node))
        {
            uint8_t rTable = GenExpr(n->table.get());
            uint8_t rA = NextRegister();

            if (auto* s = dynamic_cast<AstString*>(n->key.get()))
            {
                uint16_t sidx = AddString(s->value);
                TValue tv; tv.type = Type::STRING; tv.str = sidx;
                uint16_t kidx = AddConstant(tv);
                Emit(OperatorCode::GETFIELD, rA, kidx, rTable);
            }
            else if (auto* num = dynamic_cast<AstNumber*>(n->key.get()))
            {
                uint16_t kidx = AddConstant(TValue::Number(num->value));
                Emit(OperatorCode::GETFIELD, rA, kidx, rTable);
            }

            return rA;
        }
        else if (auto* n = dynamic_cast<AstMethodCall*>(node)) // ย้ายออกมาตรงนี้
        {
            if (n->method.empty())
            {
                // f() — call expression directly
                uint8_t rFunc = NextRegister();
                uint8_t argBase = rFunc + 1;
                for (size_t i = 0; i < n->args.size(); i++)
                    AllocatedRegisters[argBase + i] = true;

                uint8_t rObj = GenExpr(n->object.get());
                Emit(OperatorCode::MOVE, rFunc, rObj);
                FreeRegister(rObj);

                for (size_t i = 0; i < n->args.size(); i++)
                {
                    uint8_t rVal = GenExpr(n->args[i].get());
                    if (rVal != argBase + i)
                        Emit(OperatorCode::MOVE, (uint8_t)(argBase + i), rVal);
                    FreeRegister(rVal);
                }

                uint8_t rA = NextRegister();
                Emit(OperatorCode::CALL, rFunc, (uint16_t)n->args.size(), rA);

                for (size_t i = 0; i < n->args.size(); i++)
                    FreeRegister(argBase + i);
                FreeRegister(rFunc);
                return rA;
            }

            uint8_t rFunc = NextRegister();
            uint8_t argBase = rFunc + 1;
            for (size_t i = 0; i < n->args.size(); i++)
                AllocatedRegisters[argBase + i] = true;

            uint8_t rObj = GenExpr(n->object.get());
            uint16_t sidx = AddString(n->method);
            TValue tv; tv.type = Type::STRING; tv.str = sidx;
            uint16_t kidx = AddConstant(tv);
            Emit(OperatorCode::GETFIELD, rFunc, kidx, rObj);
            FreeRegister(rObj);

            for (size_t i = 0; i < n->args.size(); i++)
            {
                uint8_t rVal = GenExpr(n->args[i].get());
                if (rVal != argBase + i)
                    Emit(OperatorCode::MOVE, (uint8_t)(argBase + i), rVal);
                FreeRegister(rVal);
            }

            uint8_t rA = NextRegister();
            Emit(OperatorCode::CALL, rFunc, (uint16_t)n->args.size(), rA);

            for (size_t i = 0; i < n->args.size(); i++)
                FreeRegister(argBase + i);
            FreeRegister(rFunc);
            return rA;
        }
        else if (auto* n = dynamic_cast<AstString*>(node))
        {
            uint8_t reg = NextRegister();
            uint16_t sidx = AddString(n->value);
            TValue tv; tv.type = Type::STRING; tv.str = sidx;
            uint16_t idx = AddConstant(tv);
            Emit(OperatorCode::LOADK, reg, idx);
            return reg;
        }
        else if (auto* n = dynamic_cast<AstIdentifier*>(node))
        {
            int local = ResolveLocal(n->name);
            if (local >= 0) return (uint8_t)local;

            // เช็ค upvalue
            for (size_t i = 0; i < m_outerSymbols.size(); i++)
            {
                if (m_outerSymbols[i].first == n->name)
                {
                    // เช็คก่อนว่ามีอยู่แล้วไหม
                    for (size_t j = 0; j < m_chunk.upvalues.size(); j++)
                    {
                        if (m_chunk.upvalues[j].name == n->name)
                        {
                            uint8_t reg = NextRegister();
                            Emit(OperatorCode::GETUPVAL, reg, (uint16_t)j);
                            return reg;
                        }
                    }
                    // ไม่มี — เพิ่มใหม่
                    uint16_t uvidx = (uint16_t)m_chunk.upvalues.size();
                    m_chunk.upvalues.push_back({ n->name, m_outerSymbols[i].second, true });
                    uint8_t reg = NextRegister();
                    Emit(OperatorCode::GETUPVAL, reg, uvidx);
                    return reg;
                }
            }

            // เช็ค upvalue ของ parent
            if (m_parentOuterUpvalues)
            {
                for (size_t i = 0; i < m_parentOuterUpvalues->size(); i++)
                {
                    if ((*m_parentOuterUpvalues)[i].name == n->name)
                    {
                        // บังคับ middle capture ก่อน
                        size_t midIdx = m_outerUpvalues->size();
                        for (size_t k = 0; k < m_outerUpvalues->size(); k++)
                            if ((*m_outerUpvalues)[k].name == n->name) { midIdx = k; break; }
                        if (midIdx == m_outerUpvalues->size())
                            m_outerUpvalues->push_back({ n->name, (uint8_t)i, false });

                        // inner เอาจาก middle's upvalue
                        for (size_t j = 0; j < m_chunk.upvalues.size(); j++)
                            if (m_chunk.upvalues[j].name == n->name)
                            {
                                uint8_t reg = NextRegister();
                                Emit(OperatorCode::GETUPVAL, reg, (uint16_t)j);
                                return reg;
                            }

                        uint16_t uvidx = (uint16_t)m_chunk.upvalues.size();
                        m_chunk.upvalues.push_back({ n->name, (uint8_t)midIdx, false });
                        uint8_t reg = NextRegister();
                        Emit(OperatorCode::GETUPVAL, reg, uvidx);
                        return reg;
                    }
                }
            }

            for (size_t i = 0; i < m_parentOuterSymbols.size(); i++)
            {
                if (m_parentOuterSymbols[i].first == n->name)
                {
                    // บังคับ middle capture x จาก outer's locals
                    size_t midIdx = m_outerUpvalues ? m_outerUpvalues->size() : 0;
                    if (m_outerUpvalues)
                    {
                        for (size_t k = 0; k < m_outerUpvalues->size(); k++)
                            if ((*m_outerUpvalues)[k].name == n->name) { midIdx = k; break; }
                        if (midIdx == m_outerUpvalues->size())
                            m_outerUpvalues->push_back({ n->name, m_parentOuterSymbols[i].second, true });
                    }

                    for (size_t j = 0; j < m_chunk.upvalues.size(); j++)
                        if (m_chunk.upvalues[j].name == n->name)
                        {
                            uint8_t reg = NextRegister();
                            Emit(OperatorCode::GETUPVAL, reg, (uint16_t)j);
                            return reg;
                        }

                    uint16_t uvidx = (uint16_t)m_chunk.upvalues.size();
                    m_chunk.upvalues.push_back({ n->name, (uint8_t)midIdx, false });
                    uint8_t reg = NextRegister();
                    Emit(OperatorCode::GETUPVAL, reg, uvidx);
                    return reg;
                }
            }


            // global
            uint8_t reg = NextRegister();
            uint16_t sidx = AddString(n->name);
            TValue tv; tv.type = Type::STRING; tv.str = sidx;
            uint16_t idx = AddConstant(tv);
            Emit(OperatorCode::GETGLOBAL, reg, idx);
            return reg;
            }
        else if (auto* n = dynamic_cast<AstBinaryExpr*>(node))
        {
            if (n->op == "and")
            {
                uint8_t rL = GenExpr(n->left.get());
                uint8_t rA = NextRegister();
                int jmp = (int)m_chunk.code.size();
                Emit(OperatorCode::JMPFALSE, rL, 0);
                uint8_t rR = GenExpr(n->right.get());
                Emit(OperatorCode::MOVE, rA, rR);
                FreeRegister(rR);
                int jmpEnd = (int)m_chunk.code.size();
                Emit(OperatorCode::JMP, 0, 0);
                m_chunk.code[jmp].B = (uint16_t)(m_chunk.code.size() - jmp - 1);
                Emit(OperatorCode::MOVE, rA, rL);
                m_chunk.code[jmpEnd].B = (uint16_t)(m_chunk.code.size() - jmpEnd - 1);
                FreeRegister(rL);
                return rA;
            }
            else if (n->op == "or")
            {
                uint8_t rL = GenExpr(n->left.get());
                uint8_t rA = NextRegister();
                int jmp = (int)m_chunk.code.size();
                Emit(OperatorCode::JMPTRUE, rL, 0);
                uint8_t rR = GenExpr(n->right.get());
                Emit(OperatorCode::MOVE, rA, rR);
                FreeRegister(rR);
                int jmpEnd = (int)m_chunk.code.size();
                Emit(OperatorCode::JMP, 0, 0);
                m_chunk.code[jmp].B = (uint16_t)(m_chunk.code.size() - jmp - 1);
                Emit(OperatorCode::MOVE, rA, rL);
                m_chunk.code[jmpEnd].B = (uint16_t)(m_chunk.code.size() - jmpEnd - 1);
                FreeRegister(rL);
                return rA;
            }

            uint8_t rL = GenExpr(n->left.get());
            uint8_t rR = GenExpr(n->right.get());
            uint8_t rA = NextRegister();

            OperatorCode op;
            if (n->op == "+")  op = OperatorCode::ADD;
            else if (n->op == "-")  op = OperatorCode::SUBTRACT;
            else if (n->op == "*")  op = OperatorCode::MUL;
            else if (n->op == "/")  op = OperatorCode::DIV;
            else if (n->op == "%")  op = OperatorCode::MODULO;
            else if (n->op == "==") op = OperatorCode::EQ;
            else if (n->op == "!=") op = OperatorCode::NEQ;
            else if (n->op == "<")  op = OperatorCode::LT;
            else if (n->op == "<=") op = OperatorCode::LE;
            else if (n->op == ">")  op = OperatorCode::GT;
            else if (n->op == ">=") op = OperatorCode::GE;
            else if (n->op == "..") op = OperatorCode::CONCAT;

            Emit(op, rA, rL, rR);
            FreeRegister(rL);
            FreeRegister(rR);
            return rA;
        }
        else if (auto* n = dynamic_cast<AstUnaryExpr*>(node))
        {
            uint8_t rV = GenExpr(n->operand.get());
            uint8_t rA = NextRegister();
            if (n->op == "not") Emit(OperatorCode::NOT, rA, rV);
            else if (n->op == "-") Emit(OperatorCode::UNM, rA, rV);
            FreeRegister(rV);
            return rA;
            }
        else if (auto* n = dynamic_cast<AstFunctionCall*>(node))
        {
            // จอง funcReg และ arg slots ติดกัน
            uint8_t rFunc = NextRegister();

            // MOVE args เข้า slot ถัดจาก funcReg
            uint8_t argBase = rFunc + 1;
            for (size_t i = 0; i < n->args.size(); i++)
                AllocatedRegisters[argBase + i] = true;

            // load function
            int local = ResolveLocal(n->name);
            if (local >= 0)
                Emit(OperatorCode::MOVE, rFunc, local);
            else
            {
                uint16_t sidx = AddString(n->name);
                TValue tv; tv.type = Type::STRING; tv.str = sidx;
                uint16_t idx = AddConstant(tv);
                Emit(OperatorCode::GETGLOBAL, rFunc, idx);
            }

            for (size_t i = 0; i < n->args.size(); i++)
            {
                uint8_t rVal = GenExpr(n->args[i].get());
                if (rVal != argBase + i)
                    Emit(OperatorCode::MOVE, argBase + i, rVal);
                FreeRegister(rVal);
            }

            uint8_t rA = NextRegister();
            Emit(OperatorCode::CALL, rFunc, (uint16_t)n->args.size(), rA);

            for (size_t i = 0; i < n->args.size(); i++)
                FreeRegister(argBase + i);
            FreeRegister(rFunc);
            return rA;
            }

        return 0;
    }

    Chunk BytecodeGenerator::GenerateFunction(const std::vector<ptrNode>& statements)
    {
        m_chunk = Chunk{};
        memset(AllocatedRegisters, false, sizeof(AllocatedRegisters));

        GenBlock(statements);
        return std::move(m_chunk);
    }
}