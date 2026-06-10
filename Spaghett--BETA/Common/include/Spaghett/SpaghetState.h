// SpaghetState.h
#pragma once
#include "TValue.h"
#include "Chunk.h"
#include <cstdint>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>
#include <setjmp.h>

namespace Spaghett
{
    struct Scheduler;

    enum class StateStatus : uint8_t
    {
        None,
        Running,
        Yield,
        Finish,
        Error,
    };

    struct CallFrame
    {
        const Chunk* chunk;
        int          pc;
        uint8_t      baseReg;
        uint8_t      retReg;
        uint8_t      outerBase;
        TValue       func;
    };

    struct ProtectedFrame
    {
        size_t  callStackSize;
        uint8_t retReg;
    };

    // ของที่ทุก thread/coroutine share กัน
    struct GlobalState
    {
        std::vector<std::string>                strings;
        std::unordered_map<std::string, TValue> globals;

        uint16_t InternString(const std::string& s)
        {
            for (uint16_t i = 0; i < (uint16_t)strings.size(); i++)
                if (strings[i] == s) return i;
            strings.push_back(s);
            return (uint16_t)(strings.size() - 1);
        }
    };

    // SpaghetState = 1 thread (main หรือ coroutine ก็เป็นอันนี้เหมือนกัน)
    struct SpaghetState
    {
        GlobalState* G = nullptr;
        StateStatus                     status = StateStatus::None;
        std::array<TValue, 256>         registers;
        std::vector<CallFrame>          callStack;
        std::vector<ProtectedFrame>     protectedFrames;
        std::string                     error;

        // coroutine fields
        SpaghetState* caller = nullptr;   // thread ที่ resume เรามา
        TValue        func;               // function ที่ coroutine นี้รัน
        TValue        yieldValue;         // ค่าที่ yield ออกไป
        jmp_buf       callerJmp;          // longjmp กลับ caller ตอน yield
        Scheduler* scheduler = nullptr;

        TValue& R(uint8_t i)
        {
            uint8_t base = callStack.empty() ? 0 : callStack.back().baseReg;
            return registers[base + i];
        }

        void SetError(const std::string& msg)
        {
            error = msg;
            if (!protectedFrames.empty())
            {
                auto& pf = protectedFrames.back();
                callStack.resize(pf.callStackSize);

                registers[pf.retReg].type = Type::BOOLEAN;
                registers[pf.retReg].b = false;

                uint16_t sidx = G->InternString(msg);
                registers[pf.retReg + 1].type = Type::STRING;
                registers[pf.retReg + 1].str = sidx;

                protectedFrames.pop_back();
                status = StateStatus::Running;
            }
            else
                status = StateStatus::Error;
        }
    };

    struct SchedulerEntry
    {
        SpaghetState* co;
        double        waitUntil = 0.0;
    };

    struct Scheduler
    {
        std::vector<SchedulerEntry> queue;
        bool ticking = false;

        void Spawn(SpaghetState* co)
        {
            queue.push_back({ co, 0.0 });
        }

        void Tick(SpaghetState* mainState);
    };

    SpaghetState* OpenSpaghettState();
    void          CloseSpaghettState(SpaghetState* state);

    SpaghetState* NewThread(SpaghetState* parent, TValue func);
}