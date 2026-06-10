#pragma once
#include "../../../Common/include/Spaghett/Token.h"
#include <iostream>
#include <format>
#include <cstring>
#include <string>
#include <vector>

namespace Spaghett
{
    class Lexer
    {
    public:
        Lexer(std::string source) : m_source(source), m_size(source.size()) {}
        ~Lexer() = default;

        void Tokenize();
        void ViewTokens() const;
        std::vector<TokenWithData>&& GetTokens() { return std::move(m_tokens); }

    private:
        int m_position = 0;
        int m_line = 0;
        int m_column = 0;
        size_t m_size = 0;
        std::string m_source;

        std::vector<TokenWithData> m_tokens;

        void Consume();
        char Current() const { return m_source[m_position]; }
        char Peek(int offset = 1) const;

        void emit(TokenType token, std::string value, Location loc);
        std::string ReadWord();
        std::string ReadString();
        std::string ReadNumber();
    };
}