#include "../include/Spaghett/SpaghettLibs.h"
#include "../../VM/include/Spaghett/VirtualMachine.h"
#include <csetjmp>

namespace Spaghett
{
    void coroutine_spawn(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        TValue func = state->registers[argBase];
        if (func.type != Type::FUNCTION) return;

        SpaghetState* co = NewThread(state, func);
        co->scheduler = state->scheduler;

        if (state->scheduler)
            state->scheduler->Spawn(co);

        state->registers[retReg] = TValue::MakeCoroutine(co);
    }

    void coroutine_wait(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {

        double seconds = 0.0;
        if (nargs > 0 && state->registers[argBase].type == Type::NUMBER)
            seconds = state->registers[argBase].num;

        if (!state->caller && !state->scheduler) return;

        double until = (double)clock() / CLOCKS_PER_SEC + seconds;

        if (state->scheduler)
        {
            for (auto& entry : state->scheduler->queue)
            {
                if (entry.co == state)
                {
                    entry.waitUntil = until;
                    break;
                }
            }
        }

        state->status = StateStatus::Yield;
        longjmp(state->callerJmp, 1);
    }

    void coroutine_create(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        TValue func = state->registers[argBase];

        if (func.type != Type::FUNCTION)
        {
            state->registers[retReg] = TValue::Null();
            return;
        }
        SpaghetState* co = NewThread(state, func);
        state->registers[retReg] = TValue::MakeCoroutine(co);
    }

    void coroutine_resume(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        TValue& coVal = state->registers[argBase];

        if (coVal.type != Type::COROUTINE)
        {
            state->SetError("coroutine.resume: expected coroutine");
            return;
        }

        SpaghetState* co = coVal.co;

        if (co->status == StateStatus::Finish)
        {
            state->registers[retReg] = TValue::Null();
            return;
        }

        if (co->status == StateStatus::Running)
        {
            state->SetError("coroutine.resume: cannot resume running coroutine");
            return;
        }

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
        {
            co->status = StateStatus::Running;
        }

        co->caller = state;

        if (setjmp(co->callerJmp) == 0)
        {
            VM vm;
            vm.Execute(*co);
            co->status = StateStatus::Finish;
        }

        state->registers[retReg] = co->yieldValue;
    }

    void coroutine_yield(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        if (!state->caller)
        {
            state->SetError("coroutine.yield: cannot yield from main thread");
            return;
        }

        state->yieldValue = state->registers[argBase];
        state->status = StateStatus::Yield;

        longjmp(state->callerJmp, 1);
    }

    void coroutine_status(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        TValue& coVal = state->registers[argBase];
        if (coVal.type != Type::COROUTINE)
        {
            state->registers[retReg] = TValue::Null();
            return;
        }

        SpaghetState* co = coVal.co;

        const char* s = "dead";
        if (co->status == StateStatus::None)    s = "suspended";
        if (co->status == StateStatus::Yield)   s = "suspended";
        if (co->status == StateStatus::Running) s = "running";

        uint16_t sidx = state->G->InternString(s);
        state->registers[retReg].type = Type::STRING;
        state->registers[retReg].str = sidx;
    }

    void coroutine_wrap(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        TValue func = state->registers[argBase];
        if (func.type != Type::FUNCTION)
        {
            state->registers[retReg] = TValue::Null();
            return;
        }

        SpaghetState* co = NewThread(state, func);

        auto wrapFn = [co](SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
            {
                if (co->status == StateStatus::Finish) return;

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

                co->caller = state;

                if (setjmp(co->callerJmp) == 0)
                {
                    VM vm;
                    vm.Execute(*co);
                    co->status = StateStatus::Finish;
                }

                state->registers[retReg] = co->yieldValue;
            };

        state->registers[retReg] = TValue::MakeNativeClosure(wrapFn);
    }
}