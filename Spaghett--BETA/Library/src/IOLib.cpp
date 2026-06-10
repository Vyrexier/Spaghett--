#include "../include/Spaghett/SpaghettLibs.h"

namespace Spaghett
{
    void io_read(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        std::string input;
        std::getline(std::cin, input);

        const Chunk* chunk = state->callStack.back().chunk;
        Chunk* mchunk = const_cast<Chunk*>(chunk);

        TValue result;
        result.type = Type::STRING;
        result.str = (uint16_t)mchunk->strings.size();
        mchunk->strings.push_back(input);
        state->registers[retReg] = result;
    }

    void io_write(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        const Chunk* chunk = state->callStack.back().chunk;

        for (uint8_t i = 0; i < nargs; i++)
        {
            TValue& arg = state->registers[argBase + i];
            switch (arg.type)
            {
            case Type::NUMBER:
            {
                char temp[64];
                int len = snprintf(temp, sizeof(temp), "%.15g", arg.num);
                fwrite(temp, 1, len, stdout);
                break;
            }
            case Type::BOOLEAN:
                arg.b ? fwrite("true", 1, 4, stdout) : fwrite("false", 1, 5, stdout);
                break;
            case Type::NULL_:
                fwrite("null", 1, 4, stdout);
                break;
            case Type::STRING:
            {
                const std::string& s = chunk->strings[arg.str];
                fwrite(s.data(), 1, s.size(), stdout);
                break;
            }
            case Type::FUNCTION:
            {
                char temp[32];
                int len = snprintf(temp, sizeof(temp), "function: 0x%llX", (uintptr_t)arg.cfunc);
                fwrite(temp, 1, len, stdout);
                break;
            }
            case Type::TABLE:
            {
                char temp[32];
                int len = snprintf(temp, sizeof(temp), "table: 0x%llX", (uintptr_t)arg.table);
                fwrite(temp, 1, len, stdout);
                break;
            }
            }
        }

        fflush(stdout);
        state->registers[retReg] = TValue::Null();
    }
}