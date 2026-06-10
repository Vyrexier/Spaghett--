#include "../include/Spaghett/BytecodeGenerator.h"

namespace Spaghett
{
    uint8_t BytecodeGenerator::NextRegister()
    {
        for (int i = m_nextLocal; i < 256; i++)
        {
            if (!AllocatedRegisters[i])
            {
                AllocatedRegisters[i] = true;
                return (uint8_t)i;
            }
        }
        throw std::runtime_error("Out of registers");
    }

    uint8_t BytecodeGenerator::NextLocalRegister()
    {
        uint8_t reg = m_nextLocal++;
        AllocatedRegisters[reg] = true;
        return reg;
    }

    void BytecodeGenerator::FreeRegister(uint8_t reg)
    {
        if (!IsLocalRegister(reg))
            AllocatedRegisters[reg] = false;
    }

    bool BytecodeGenerator::IsLocalRegister(uint8_t reg)
    {
        for (auto& [name, r] : m_symbolTable)
            if (r == reg) return true;
        return false;
    }

    uint8_t BytecodeGenerator::DeclareLocal(const std::string& name)
    {
        uint8_t reg = NextRegister();
        m_symbolTable.push_back({ name, reg });
        return reg;
    }

    int BytecodeGenerator::ResolveLocal(const std::string& name)
    {
        for (auto it = m_symbolTable.rbegin(); it != m_symbolTable.rend(); ++it)
            if (it->first == name) return it->second;
        return -1;
    }

    uint16_t BytecodeGenerator::AddConstant(TValue val)
    {
        for (int i = 0; i < (int)m_chunk.constants.size(); i++)
        {
            auto& c = m_chunk.constants[i];
            if (c.type != val.type) continue;
            switch (val.type)
            {
            case Type::NUMBER:  if (c.num == val.num) return (uint16_t)i; break;
            case Type::BOOLEAN: if (c.b == val.b)     return (uint16_t)i; break;
            case Type::NULL_:                          return (uint16_t)i;
            case Type::STRING:  if (c.str == val.str) return (uint16_t)i; break;
            }
        }
        m_chunk.constants.push_back(val);
        return (uint16_t)(m_chunk.constants.size() - 1);
    }

    uint16_t BytecodeGenerator::AddString(const std::string& str)
    {
        if (m_G)
            return m_G->InternString(str);

        for (int i = 0; i < (int)m_chunk.strings.size(); i++)
            if (m_chunk.strings[i] == str) return (uint16_t)i;
        m_chunk.strings.push_back(str);
        return (uint16_t)(m_chunk.strings.size() - 1);
    }

    void BytecodeGenerator::Emit(OperatorCode op, uint8_t A, uint16_t B, uint8_t C)
    {
        m_chunk.code.push_back({ op, A, B, C });
    }

    void BytecodeGenerator::Emit(OperatorCode op, uint8_t A, uint16_t B)
    {
        m_chunk.code.push_back({ op, A, B, 0 });
    }
}