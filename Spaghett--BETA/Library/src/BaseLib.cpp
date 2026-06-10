#include "../include/Spaghett/SpaghettLibs.h"

namespace Spaghett
{
    void print(SpaghetState* state, uint8_t funcReg, uint8_t nargs, uint8_t retReg)
    {
        const Chunk* chunk = state->callStack.back().chunk;
        static char buffer[8192];
        char* ptr = buffer;
        const char* end = buffer + sizeof(buffer) - 128;

        for (uint8_t i = 0; i < nargs; i++)
        {
            TValue& arg = state->registers[funcReg + i];

            if (ptr > end) {
                fwrite(buffer, 1, ptr - buffer, stdout);
                ptr = buffer;
            }

            switch (arg.type)
            {
            case Type::NUMBER:
            {
                double v = arg.num;
                char temp[64];
                int len = snprintf(temp, sizeof(temp), "%.15f", v);
                while (len > 0 && temp[len - 1] == '0') temp[--len] = '\0';
                if (len > 0 && temp[len - 1] == '.') temp[--len] = '\0';
                ptr += snprintf(ptr, 64, "%s ", temp);
                break;
            }
            case Type::BOOLEAN:
                if (arg.b) { memcpy(ptr, "true ", 5); ptr += 5; }
                else { memcpy(ptr, "false ", 6); ptr += 6; }
                break;
            case Type::NULL_:
                memcpy(ptr, "null ", 5); ptr += 5;
                break;
            case Type::STRING:
            {
                const Chunk* chunk = state->callStack.back().chunk;
                const std::string& s = chunk->strings[arg.str];
                if (s.size() < 4096) {
                    memcpy(ptr, s.data(), s.size());
                    ptr += s.size();
                    *ptr++ = ' ';
                }
                break;
            }
            case Type::FUNCTION:
            {
                char temp[32];
                int len = snprintf(temp, sizeof(temp), "function: 0x%llX ", (uintptr_t)arg.cfunc);
                memcpy(ptr, temp, len);
                ptr += len;
                break;
            }
            case Type::TABLE:
            {
                char temp[32];
                int len = snprintf(temp, sizeof(temp), "table: 0x%llX ", (uintptr_t)arg.table);
                memcpy(ptr, temp, len);
                ptr += len;
                break;
            }
            }
        }

        fwrite(buffer, 1, ptr - buffer, stdout);
        fflush(stdout);

        state->registers[retReg].type = Type::NULL_;
    }

    void printl(SpaghetState* state, uint8_t funcReg, uint8_t nargs, uint8_t retReg)
    {
        const Chunk* chunk = state->callStack.back().chunk;
        static char buffer[8192];
        char* ptr = buffer;
        const char* end = buffer + sizeof(buffer) - 128;

        for (uint8_t i = 0; i < nargs; i++)
        {
            TValue& arg = state->registers[funcReg + i];

            if (ptr > end) {
                fwrite(buffer, 1, ptr - buffer, stdout);
                ptr = buffer;
            }

            switch (arg.type)
            {
            case Type::NUMBER:
            {
                double v = arg.num;
                char temp[64];
                int len = snprintf(temp, sizeof(temp), "%.15f", v);
                while (len > 0 && temp[len - 1] == '0') temp[--len] = '\0';
                if (len > 0 && temp[len - 1] == '.') temp[--len] = '\0';
                ptr += snprintf(ptr, 64, "%s ", temp);
                break;
            }
            case Type::BOOLEAN:
                if (arg.b) { memcpy(ptr, "true ", 5); ptr += 5; }
                else { memcpy(ptr, "false ", 6); ptr += 6; }
                break;
            case Type::NULL_:
                memcpy(ptr, "null ", 5); ptr += 5;
                break;
            case Type::STRING:
            {
                const Chunk* chunk = state->callStack.back().chunk;
                const std::string& s = chunk->strings[arg.str];
                if (s.size() < 4096) {
                    memcpy(ptr, s.data(), s.size());
                    ptr += s.size();
                    *ptr++ = ' ';
                }
                break;
            }
            case Type::FUNCTION:
            {
                char temp[32];
                int len = snprintf(temp, sizeof(temp), "0x%llX ", (uintptr_t)arg.cfunc);
                memcpy(ptr, temp, len);
                ptr += len;
                break;
            }
            case Type::TABLE:
            {
                char temp[32];
                int len = snprintf(temp, sizeof(temp), "0x%llX ", (uintptr_t)arg.table);
                memcpy(ptr, temp, len);
                ptr += len;
                break;
            }
            }
        }

        *ptr++ = '\n';
        fwrite(buffer, 1, ptr - buffer, stdout);
        fflush(stdout);

        state->registers[retReg].type = Type::NULL_;
    }

    void tostring(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        TValue& arg = state->registers[argBase];
        TValue result;
        result.type = Type::STRING;

        const Chunk* chunk = state->callStack.back().chunk;
        // cast away const เพื่อ push string ใหม่
        Chunk* mchunk = const_cast<Chunk*>(chunk);

        char buf[64];
        switch (arg.type)
        {
        case Type::NUMBER:
        {
            int len = snprintf(buf, sizeof(buf), "%.15g", arg.num);
            result.str = (uint16_t)mchunk->strings.size();
            mchunk->strings.push_back(std::string(buf, len));
            break;
        }
        case Type::BOOLEAN:
            result.str = (uint16_t)mchunk->strings.size();
            mchunk->strings.push_back(arg.b ? "true" : "false");
            break;
        case Type::NULL_:
            result.str = (uint16_t)mchunk->strings.size();
            mchunk->strings.push_back("null");
            break;
        case Type::STRING:
            result = arg;
            break;
        case Type::FUNCTION:
            result.str = (uint16_t)mchunk->strings.size();
            mchunk->strings.push_back(std::format("0x{:016X}", (uintptr_t)arg.cfunc));
            break;
        case Type::TABLE:
            result.str = (uint16_t)mchunk->strings.size();
            mchunk->strings.push_back(std::format("0x{:016X}", (uintptr_t)arg.table));
            break;
        default:
            result.str = (uint16_t)mchunk->strings.size();
            mchunk->strings.push_back("?");
            break;
        }

        state->registers[retReg] = result;
    }

    void tonumber(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        TValue& arg = state->registers[argBase];

        switch (arg.type)
        {
        case Type::NUMBER:
            state->registers[retReg] = arg;
            break;
        case Type::STRING:
        {
            const Chunk* chunk = state->callStack.back().chunk;
            const std::string& s = chunk->strings[arg.str];
            try {
                state->registers[retReg] = TValue::Number(std::stod(s));
            }
            catch (...) {
                state->registers[retReg] = TValue::Null();
            }
            break;
        }
        default:
            state->registers[retReg] = TValue::Null();
            break;
        }
    }

    void pcall(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        TValue func = state->registers[argBase];
        if (func.type != Type::FUNCTION)
        {
            state->registers[retReg].type = Type::BOOLEAN;
            state->registers[retReg].b = false;
            return;
        }

        ProtectedFrame pf;
        pf.callStackSize = state->callStack.size();
        pf.retReg = retReg;
        state->protectedFrames.push_back(pf);

        // push call frame ของ function ที่จะ call
        auto& frame = state->callStack.back();
        CallFrame newFrame;
        newFrame.chunk = func.chunk;
        newFrame.pc = 0;
        newFrame.baseReg = argBase + 1;
        newFrame.retReg = retReg;
        newFrame.outerBase = frame.baseReg;
        newFrame.func = func;

        // copy args
        for (uint8_t i = 1; i < nargs; i++)
            state->registers[argBase + i] = state->registers[argBase + i];

        state->callStack.push_back(newFrame);

        // on success — retReg = true
        // VM จะ run ต่อ แล้วตอน RETURN จะ pop frame
        // ต้องรู้ว่า return จาก pcall frame แล้วให้ set true
        // ทำโดย mark retReg ไว้ใน ProtectedFrame
    }

    void error(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg)
    {
        const Chunk* chunk = state->callStack.back().chunk;
        TValue& arg = state->registers[argBase];

        std::string msg = "error";
        if (arg.type == Type::STRING)
            msg = chunk->strings[arg.str];
        else if (arg.type == Type::NUMBER)
            msg = std::to_string(arg.num);

        state->SetError(msg);
    }
}