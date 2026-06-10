#include "../include/Spaghett/Lexer.h"

namespace Spaghett
{
    char Lexer::Peek(int offset) const
    {
        int idx = m_position + offset;
        if (idx < 0 || idx >= (int)m_size) return '\0';
        return m_source[idx];
    }

    void Lexer::ViewTokens() const
    {
        for (auto& token : m_tokens)
        {
            std::cout << std::format("[{:<15}] value = {:<15} line={:<5} col={:<5}\n",
                TokenTypeToString(token.type),
                token.value,
                token.location.begin.line,
                token.location.begin.column);
        }
    }

    void Lexer::emit(TokenType token, std::string value, Location loc)
    {
        m_tokens.push_back({ token , value , loc});
    }

    void Lexer::Consume()
    {
        if (m_position < m_size)
        {
            if (m_source[m_position] == '\n')
            {
                m_line++;
                m_column = 0;
            }
            else
            {
                m_column++;
            }
            m_position++;
        }
    }

    std::string Lexer::ReadWord()
    {
        std::string word;
        while (m_position < m_size && (std::isalnum(m_source[m_position]) || m_source[m_position] == '_'))
        {
            word += m_source[m_position];
            Consume();
        }
        return word;
    }

    std::string Lexer::ReadString()
    {
        std::string str;
        Consume();
        while (m_position < m_size &&
            m_source[m_position] != '"' &&
            m_source[m_position] != '\n')
        {
            // escape sequence
            if (m_source[m_position] == '\\')
            {
                Consume();
                switch (m_source[m_position])
                {
                case 'n':  str += '\n'; break;
                case 't':  str += '\t'; break;
                case '"':  str += '"';  break;
                case '\\': str += '\\'; break;
                default:   str += m_source[m_position]; break;
                }
            }
            else
            {
                str += m_source[m_position];
            }
            Consume();
        }

        Consume();
        return str;
    }

    std::string Lexer::ReadNumber()
    {
        std::string num;

        while (m_position < m_size &&
            (std::isdigit(m_source[m_position]) || m_source[m_position] == '_'))
        {
            if (m_source[m_position] != '_')
                num += m_source[m_position];
            Consume();
        }

        // decimal
        if (m_position < m_size && m_source[m_position] == '.')
        {
            num += '.';
            Consume();
            while (m_position < m_size &&
                (std::isdigit(m_source[m_position]) || m_source[m_position] == '_'))
            {
                if (m_source[m_position] != '_')
                    num += m_source[m_position];
                Consume();
            }
        }

        // exponent
        if (m_position < m_size &&
            (m_source[m_position] == 'e' || m_source[m_position] == 'E'))
        {
            num += m_source[m_position];
            Consume();

            if (m_position < m_size &&
                (m_source[m_position] == '+' || m_source[m_position] == '-'))
            {
                num += m_source[m_position];
                Consume();
            }

            while (m_position < m_size && std::isdigit(m_source[m_position]))
            {
                num += m_source[m_position];
                Consume();
            }
        }

        return num;
    }
}