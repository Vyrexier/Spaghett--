#pragma once
#include <cstdint>
#include <string>

namespace Spaghett
{
    enum class OperatorCode
    {
        // Load
        LOADK,      // load constant
        MOVE,

        CLOSURE,

        FORPREP,  // setup table iterator
        FORNEXT,  // next key/value, jump if done

        CONCAT,

        // Arithmetic
        ADD,
        SUBTRACT,
        MUL,
        DIV,
        MODULO,
        UNM,        // unary minus -x

        // Logic
        NOT,        // not x

        // Comparison
        EQ,         // ==
        NEQ,        // !=
        LT,         // 
        LE,         // <=
        GT,         // >
        GE,         // >=

        SETGLOBAL,
        GETGLOBAL,

        GETUPVAL,
        SETUPVAL,

        // Table
        NEWTABLE,
        SETFIELD,
        GETFIELD,

        // Function
        CALL,
        RETURN,

        // Jump
        JMP,
        JMPFALSE,
        JMPTRUE,
    };

    inline std::string OpCodeToString(OperatorCode op)
    {
        switch (op)
        {
        case OperatorCode::LOADK:      return "LOADK";
        case OperatorCode::MOVE:       return "MOVE";
        case OperatorCode::CLOSURE:       return "CLOSURE";
        case OperatorCode::CONCAT: return "CONCAT";

        case OperatorCode::ADD:        return "ADD";
        case OperatorCode::SUBTRACT:   return "SUBTRACT";
        case OperatorCode::MUL:        return "MUL";
        case OperatorCode::DIV:        return "DIV";
        case OperatorCode::MODULO:     return "MODULO";
        case OperatorCode::UNM:        return "UNM";
        case OperatorCode::FORPREP:        return "FORPREP"; // setup table iterator
        case OperatorCode::FORNEXT:        return "FORNEXT";// next key/value, jump if done
        case OperatorCode::NOT:        return "NOT";

        case OperatorCode::EQ:         return "EQ";
        case OperatorCode::NEQ:        return "NEQ";
        case OperatorCode::LT:         return "LT";
        case OperatorCode::LE:         return "LE";
        case OperatorCode::GT:         return "GT";
        case OperatorCode::GE:         return "GE";

        case OperatorCode::GETUPVAL: return "GETUPVAL";
        case OperatorCode::SETUPVAL: return "SETUPVAL";

        case OperatorCode::SETGLOBAL:  return "SETGLOBAL";
        case OperatorCode::GETGLOBAL:  return "GETGLOBAL";

        case OperatorCode::NEWTABLE:   return "NEWTABLE";
        case OperatorCode::SETFIELD:   return "SETFIELD";
        case OperatorCode::GETFIELD:   return "GETFIELD";

        case OperatorCode::CALL:       return "CALL";
        case OperatorCode::RETURN:     return "RETURN";

        case OperatorCode::JMP:        return "JMP";
        case OperatorCode::JMPFALSE:   return "JMPFALSE";
        case OperatorCode::JMPTRUE:    return "JMPTRUE";

        default:                       return "UNKNOWN";
        }
    }

    struct Instruction
    {
        OperatorCode op;
        uint8_t A;
        uint16_t B;
        uint8_t C;
    };
}