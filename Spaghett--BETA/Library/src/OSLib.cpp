#include "../include/Spaghett/SpaghettLibs.h"

namespace Spaghett
{
    void os_time(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        auto now = std::chrono::system_clock::now();
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        state->registers[retReg] = TValue::Number((double)secs);
    }

    void os_clock(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        auto now = std::chrono::steady_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
        state->registers[retReg] = TValue::Number((double)us / 1000000.0);
    }

    void os_date(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        char buf[64];
        struct tm tm_info;
        localtime_s(&tm_info, &t);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);

        const Chunk* chunk = state->callStack.back().chunk;
        Chunk* mchunk = const_cast<Chunk*>(chunk);
        TValue result;
        result.type = Type::STRING;
        result.str = (uint16_t)mchunk->strings.size();
        mchunk->strings.push_back(buf);
        state->registers[retReg] = result;
    }
}