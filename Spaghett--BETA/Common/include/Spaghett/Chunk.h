#pragma once
#include "Instruction.h"
#include <iostream>
#include "TValue.h"
#include <format>
#include <string>
#include <vector>

namespace Spaghett
{

    struct UpvalueDesc
    {
        std::string name;
        uint8_t index;
        bool isLocal;
    };

    struct Chunk
    {
        std::string name;
        int params = 0;
        std::vector<Instruction> code;
        std::vector<TValue> constants;
        std::vector<std::string> strings;
        std::vector<Chunk*> subchunks;
        std::vector<UpvalueDesc> upvalues;

        void Dump(const std::vector<std::string>* gstrings = nullptr) const
        {
            std::cout << std::format("== {} ==\n", name.empty() ? "main" : name);

            for (int i = 0; i < (int)code.size(); i++)
            {
                auto& ins = code[i];
                std::string line = std::format("{:4d} {:12}", i, OpCodeToString(ins.op));

                switch (ins.op)
                {
                case OperatorCode::LOADK:
                    line += std::format("R{} K{}", ins.A, ins.B);
                    break;
                case OperatorCode::CLOSURE:
                    line += std::format("R{} chunk[{}]", ins.A, ins.B);
                    break;
                case OperatorCode::GETGLOBAL:
                case OperatorCode::SETGLOBAL:
                    line += std::format("R{} K{}", ins.A, ins.B);
                    break;
                case OperatorCode::GETUPVAL:
                case OperatorCode::SETUPVAL:
                    line += std::format("R{} UV{}", ins.A, ins.B);
                    break;
                case OperatorCode::MOVE:
                case OperatorCode::NOT:
                case OperatorCode::UNM:
                    line += std::format("R{} R{}", ins.A, ins.B);
                    break;
                case OperatorCode::ADD:
                case OperatorCode::SUBTRACT:
                case OperatorCode::MUL:
                case OperatorCode::DIV:
                case OperatorCode::MODULO:
                case OperatorCode::EQ:
                case OperatorCode::NEQ:
                case OperatorCode::LT:
                case OperatorCode::LE:
                case OperatorCode::GT:
                case OperatorCode::GE:
                    line += std::format("R{} R{} R{}", ins.A, ins.B, ins.C);
                    break;
                case OperatorCode::JMPFALSE:
                case OperatorCode::JMPTRUE:
                    line += std::format("R{} {}", ins.A, ins.B);
                    break;
                case OperatorCode::JMP:
                    line += std::format("{}", (int16_t)ins.B);
                    break;
                case OperatorCode::CALL:
                    line += std::format("R{} {} R{}", ins.A, ins.B, ins.C);
                    break;
                case OperatorCode::RETURN:
                    line += std::format("R{}", ins.A);
                    break;
                case OperatorCode::SETFIELD:
                    line += std::format("R{} K{} R{}", ins.A, ins.B, ins.C);
                    break;
                case OperatorCode::GETFIELD:
                    line += std::format("R{} R{} K{}", ins.A, ins.C, ins.B);
                    break;
                default:
                    line += std::format("R{} {} {}", ins.A, ins.B, ins.C);
                    break;
                }

                std::cout << line << "\n";
            }

            std::cout << "\nConstants:\n";
            for (int i = 0; i < (int)constants.size(); i++)
            {
                auto& tv = constants[i];
                switch (tv.type)
                {
                case Type::NUMBER:  std::cout << std::format("  K{} = {}\n", i, tv.num); break;
                case Type::BOOLEAN: std::cout << std::format("  K{} = {}\n", i, tv.b ? "true" : "false"); break;
                case Type::NULL_:   std::cout << std::format("  K{} = null\n", i); break;
                case Type::STRING:
                {
                    const std::string& s = (gstrings && tv.str < gstrings->size())
                        ? (*gstrings)[tv.str]
                        : (tv.str < strings.size() ? strings[tv.str] : "?");
                    std::cout << std::format("  K{} = '{}'\n", i, s);
                    break;
                }
                }
            }

            for (int i = 0; i < (int)subchunks.size(); i++)
            {
                std::cout << std::format("\n-- subchunk[{}] --\n", i);
                subchunks[i]->Dump(gstrings);
            }
        }
    };
}