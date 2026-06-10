#pragma once
#include "../include/Spaghett/VirtualMachine.h"


namespace Spaghett
{

    void Scheduler::Tick(SpaghetState* mainState)
    {
        if (ticking) return;
        ticking = true;
        double now = (double)GetTime();

        for (auto& entry : queue)
        {
            if (now < entry.waitUntil) continue;
            if (entry.co->status == StateStatus::Finish) continue;

            SpaghetState* co = entry.co;

            if (co->status == StateStatus::None)
            {
                CallFrame frame;
                frame.chunk = co->func.chunk;
                frame.pc = 0;
                frame.baseReg = 0;
                frame.retReg = 0;
                frame.outerBase = 0;
                frame.func = co->func;
                co->callStack.push_back(frame);
                co->status = StateStatus::Running;
            }
            else if (co->status == StateStatus::Yield)
                co->status = StateStatus::Running;

            co->caller = mainState;

            if (setjmp(co->callerJmp) == 0)
            {
                VM::Execute(*co);
                co->status = StateStatus::Finish;
            }
        }

        queue.erase(
            std::remove_if(queue.begin(), queue.end(),
                [](const SchedulerEntry& e) {
                    return e.co->status == StateStatus::Finish;
                }),
            queue.end()
        );

        if (!queue.empty())
            mainState->status = StateStatus::Running;

        ticking = false;
    }

    void VM::Load(SpaghetState& state, const Chunk& chunk)
    {
        state.status = StateStatus::Running;
        state.callStack.push_back({ &chunk, 0, 0, 0, 0, TValue::Null() });
    }

    void VM::Call(SpaghetState& state, int nargs, int nresults)
    {
        Execute(state);
    }

    void VM::Execute(SpaghetState& state)
    {
        while (state.status == StateStatus::Running)
        {
            auto& frame = state.callStack.back();
            if (frame.pc >= (int)frame.chunk->code.size())
            {
                state.status = StateStatus::Finish;

                if (state.scheduler && !state.scheduler->ticking
                    && !state.scheduler->queue.empty())
                {
                    state.status = StateStatus::Running;
                    while (!state.scheduler->queue.empty())
                        state.scheduler->Tick(&state);
                }
                break;
            }

            const Instruction& ins = frame.chunk->code[frame.pc++];

            switch (ins.op)
            {
            case OperatorCode::LOADK:
                state.R(ins.A) = frame.chunk->constants[ins.B];
                break;

            case OperatorCode::MOVE:
                state.R(ins.A) = state.R(ins.B);
                break;

            case OperatorCode::ADD:
                state.R(ins.A).type = Type::NUMBER;
                state.R(ins.A).num = state.R(ins.B).num + state.R(ins.C).num;
                break;

            case OperatorCode::SUBTRACT:
                state.R(ins.A).type = Type::NUMBER;
                state.R(ins.A).num = state.R(ins.B).num - state.R(ins.C).num;
                break;

            case OperatorCode::MUL:
                state.R(ins.A).type = Type::NUMBER;
                state.R(ins.A).num = state.R(ins.B).num * state.R(ins.C).num;
                break;

            case OperatorCode::DIV:
                state.R(ins.A).type = Type::NUMBER;
                state.R(ins.A).num = state.R(ins.B).num / state.R(ins.C).num;
                break;

            case OperatorCode::MODULO:
                state.R(ins.A).type = Type::NUMBER;
                state.R(ins.A).num = std::fmod(state.R(ins.B).num, state.R(ins.C).num);
                break;

            case OperatorCode::UNM:
                state.R(ins.A).type = Type::NUMBER;
                state.R(ins.A).num = -state.R(ins.B).num;
                break;

            case OperatorCode::NOT:
                state.R(ins.A).type = Type::BOOLEAN;
                state.R(ins.A).b = !state.R(ins.B).b;
                break;

            case OperatorCode::EQ:
                state.R(ins.A).type = Type::BOOLEAN;
                state.R(ins.A).b = state.R(ins.B).num == state.R(ins.C).num;
                break;

            case OperatorCode::NEQ:
                state.R(ins.A).type = Type::BOOLEAN;
                state.R(ins.A).b = state.R(ins.B).num != state.R(ins.C).num;
                break;

            case OperatorCode::LT:
                state.R(ins.A).type = Type::BOOLEAN;
                state.R(ins.A).b = state.R(ins.B).num < state.R(ins.C).num;
                break;

            case OperatorCode::LE:
                state.R(ins.A).type = Type::BOOLEAN;
                state.R(ins.A).b = state.R(ins.B).num <= state.R(ins.C).num;
                break;

            case OperatorCode::GT:
                state.R(ins.A).type = Type::BOOLEAN;
                state.R(ins.A).b = state.R(ins.B).num > state.R(ins.C).num;
                break;

            case OperatorCode::GE:
                state.R(ins.A).type = Type::BOOLEAN;
                state.R(ins.A).b = state.R(ins.B).num >= state.R(ins.C).num;
                break;

            case OperatorCode::GETGLOBAL:
            {
                const std::string& name = frame.chunk->strings[frame.chunk->constants[ins.B].str];
                auto it = state.G->globals.find(name);
                if (it != state.G->globals.end())
                    state.R(ins.A) = it->second;
                else
                    state.SetError("undefined global '" + name + "'");
                break;
            }
            case OperatorCode::SETGLOBAL:
            {
                const std::string& name = frame.chunk->strings[frame.chunk->constants[ins.B].str];
                state.G->globals[name] = state.R(ins.A);
                break;
            }
            case OperatorCode::FORPREP:
                state.R(ins.B).type = Type::NUMBER;
                state.R(ins.B).num = 0;
                break;

            case OperatorCode::FORNEXT:
            {
                Table* t = state.R(ins.A).table;
                int idx = (int)state.R(ins.B).num;
                uint8_t rDone = ins.C - 1;

                auto it = t->hash.begin();
                std::advance(it, idx);

                if (it == t->hash.end())
                {
                    state.R(rDone).type = Type::BOOLEAN;
                    state.R(rDone).b = false;
                    break;
                }

                state.R(rDone).type = Type::BOOLEAN;
                state.R(rDone).b = true;

                // key
                uint16_t sidx = state.G->InternString(it->first);
                state.R(ins.C).type = Type::STRING;
                state.R(ins.C).str = sidx;

                // value
                state.R(ins.C + 1) = it->second;

                // increment
                state.R(ins.B).num = idx + 1;
                break;
            }
            case OperatorCode::JMP:
                frame.pc += (int16_t)ins.B;
                break;

            case OperatorCode::JMPFALSE:
                if (!state.R(ins.A).b)
                    frame.pc += (int16_t)ins.B;
                break;

            case OperatorCode::JMPTRUE:
                if (state.R(ins.A).b)
                    frame.pc += (int16_t)ins.B;
                break;
            case OperatorCode::CLOSURE:
            {
                TValue func = TValue::MakeFunction(frame.chunk->subchunks[ins.B]);
                Chunk* sub = frame.chunk->subchunks[ins.B];
                func.upvalues = new std::vector<std::shared_ptr<UpvalueBox>>();

                for (auto& uv : sub->upvalues)
                {
                    if (uv.isLocal)
                    {
                        auto box = std::make_shared<UpvalueBox>();
                        box->value = state.registers[frame.baseReg + uv.index];
                        func.upvalues->push_back(box);
                    }
                    else
                    {
                        if (frame.func.upvalues && uv.index < frame.func.upvalues->size())
                            func.upvalues->push_back((*frame.func.upvalues)[uv.index]);
                    }
                }

                state.R(ins.A) = func;
                break;
            }
            case OperatorCode::RETURN:
            {
                uint8_t nret = ins.B == 0 ? 1 : ins.B;
                CallFrame finished = state.callStack.back();
                state.callStack.pop_back();

                if (!state.protectedFrames.empty() &&
                    state.callStack.size() == state.protectedFrames.back().callStackSize)
                {
                    auto& pf = state.protectedFrames.back();
                    state.registers[pf.retReg].type = Type::BOOLEAN;
                    state.registers[pf.retReg].b = true;
                    state.protectedFrames.pop_back();
                }

                if (state.callStack.empty())
                {
                    state.status = StateStatus::Finish;

                    if (state.scheduler && !state.scheduler->queue.empty())
                    {
                        state.status = StateStatus::Running;
                        while (!state.scheduler->queue.empty())
                            state.scheduler->Tick(&state);
                    }
                }
                else
                {
                    auto& callerFrame = state.callStack.back();
                    for (uint8_t i = 0; i < nret; i++)
                        state.registers[callerFrame.baseReg + finished.retReg + i] = state.registers[finished.baseReg + ins.A + i];
                }
                break;
            }
            case OperatorCode::NEWTABLE:
                state.R(ins.A) = TValue::MakeTable();
                break;

            case OperatorCode::SETFIELD:
            {
                Table* t = state.R(ins.A).table;
                const std::string& key = frame.chunk->strings[frame.chunk->constants[ins.B].str];
                t->hash[key] = state.R(ins.C);
                break;
            }
            case OperatorCode::CONCAT:
            {
                printf("CONCAT A=%d B=%d C=%d\n", ins.A, ins.B, ins.C);

                TValue vB = state.R(ins.B);
                TValue vC = state.R(ins.C);
                printf("  vB.type=%d vB.str=%d\n", (int)vB.type, vB.str);
                printf("  vC.type=%d vC.str=%d\n", (int)vC.type, vC.str);

                const Chunk* chunk = frame.chunk;

                auto toString = [&](TValue v) -> std::string {
                    if (v.type == Type::STRING) {
                        printf("  toString: str=%d chunk_strings_size=%zu G_strings_size=%zu\n",
                            v.str, frame.chunk->strings.size(), state.G->strings.size());
                        if (v.str < frame.chunk->strings.size())
                            printf("  -> chunk->strings[%d] = '%s'\n", v.str, frame.chunk->strings[v.str].c_str());
                            return frame.chunk->strings[v.str];
                        // fallback G->strings (สำหรับ string ที่ InternString สร้าง)
                        if (v.str < state.G->strings.size())
                            return state.G->strings[v.str];
                        return "";
                    }
                    if (v.type == Type::NUMBER) {
                        double n = v.num;
                        if (n == (double)(int)n) return std::to_string((int)n);
                        return std::to_string(n);
                    }
                    return "";
                    };

                std::string result = toString(vB) + toString(vC);
                uint16_t sidx = state.G->InternString(result);
                state.R(ins.A).type = Type::STRING;
                state.R(ins.A).str = sidx;

                printf("  result='%s' interned_at=%d\n", result.c_str(), sidx);
                printf("  G->strings size=%zu\n", state.G->strings.size());
                break;
            }
            case OperatorCode::GETFIELD:
            {
                Table* t = state.R(ins.C).table;
                const std::string& key = frame.chunk->strings[frame.chunk->constants[ins.B].str];
                auto it = t->hash.find(key);
                if (it != t->hash.end())
                    state.R(ins.A) = it->second;
                else
                    state.R(ins.A) = TValue::Null();
                break;
            }
            case OperatorCode::SETUPVAL:
            {
                (*frame.func.upvalues)[ins.B]->value = state.R(ins.A);
                break;
            }
            case OperatorCode::GETUPVAL:
            {
                state.R(ins.A) = (*frame.func.upvalues)[ins.B]->value;
                break;
            }
            case OperatorCode::CALL:
            {
                uint8_t funcReg = ins.A;
                uint8_t nargs = (uint8_t)ins.B;
                uint8_t retReg = ins.C;

                TValue func = state.R(funcReg);
                if (func.type == Type::FUNCTION)
                {
                    uint8_t argBase = funcReg + 1;
                    if (func.isNative)
                    {
                        uint8_t absArgBase = frame.baseReg + argBase;
                        uint8_t absRetReg = frame.baseReg + retReg;

                        if (func.isClosure)
                            (*func.nativeClosure)(&state, absArgBase, nargs, absRetReg);
                        else
                            func.cfunc(&state, absArgBase, nargs, absRetReg);
                    }
                    else
                    {

                        uint8_t newBase = argBase;

                        CallFrame newFrame;
                        newFrame.chunk = func.chunk;
                        newFrame.pc = 0;
                        newFrame.baseReg = newBase;
                        newFrame.retReg = retReg;

                        for (uint8_t i = 0; i < nargs; i++)
                            state.registers[newBase + i] = state.registers[argBase + i];

                        newFrame.outerBase = frame.baseReg;
                        newFrame.func = func;

                        state.callStack.push_back(newFrame);
                    }
                }
                else
                    state.SetError("attempt to call a non-function");
                break;
            }
            default:
                state.SetError("unknown opcode");
                break;
            }
        }
    }
}