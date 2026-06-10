#include "../include/Spaghett/Lexer.h"

namespace Spaghett
{
    void Lexer::Tokenize()
    {
        while (m_position < m_size)
        {
            char c = Current();

            if (std::isspace(c)) { Consume(); continue; }

            Position begin(m_line, m_column);

            // Comment
            if (c == '-' && Peek() == '-')
            {
                while (m_position < m_size && Current() != '\n')
                    Consume();
                continue;
            }

            // String
            if (c == '"')
            {
                std::string str = ReadString();
                emit(TokenType::String, str, Location(begin, Position(m_line, m_column)));
                continue;
            }

            // Concat
            if (c == '.' && Peek() == '.')
            {
                Consume(); Consume();
                emit(TokenType::Concat, "..", Location(begin, Position(m_line, m_column)));
                continue;
            }

            // Identifier / Keyword
            if (std::isalpha(c) || c == '_')
            {
                std::string word = ReadWord();
                TokenType type = IsReservedKeyWord(word);
                if (type == TokenType::Unknown)
                    type = TokenType::Identifier;
                emit(type, word, Location(begin, Position(m_line, m_column)));
                continue;
            }

            // Number
            if (std::isdigit(c) || (c == '.' && std::isdigit(Peek())))
            {
                std::string num = ReadNumber();
                emit(TokenType::Number, num, Location(begin, Position(m_line, m_column)));
                continue;
            }

            Consume();
            switch (c)
            {
            case '+':
                if (Current() == '+') { Consume(); emit(TokenType::Increment, "++", Location(begin, Position(m_line, m_column))); }
                else if (Current() == '=') { Consume(); emit(TokenType::PlusEqual, "+=", Location(begin, Position(m_line, m_column))); }
                else emit(TokenType::Plus, "+", Location(begin, Position(m_line, m_column)));
                break;
            case '-':
                if (Current() == '-') { Consume(); emit(TokenType::Decrement, "--", Location(begin, Position(m_line, m_column))); }
                else if (Current() == '=') { Consume(); emit(TokenType::MinusEqual, "-=", Location(begin, Position(m_line, m_column))); }
                else emit(TokenType::Minus, "-", Location(begin, Position(m_line, m_column)));
                break;
            case '*':
                if (Current() == '=') { Consume(); emit(TokenType::MultiplyEqual, "*=", Location(begin, Position(m_line, m_column))); }
                else emit(TokenType::Multiply, "*", Location(begin, Position(m_line, m_column)));
                break;
            case '/':
                if (Current() == '=') { Consume(); emit(TokenType::DivideEqual, "/=", Location(begin, Position(m_line, m_column))); }
                else emit(TokenType::Divide, "/", Location(begin, Position(m_line, m_column)));
                break;
            case '=':
                if (Current() == '=') { Consume(); emit(TokenType::EqualEqual, "==", Location(begin, Position(m_line, m_column))); }
                else emit(TokenType::Equal, "=", Location(begin, Position(m_line, m_column)));
                break;
            case '!':
                if (Current() == '=') { Consume(); emit(TokenType::NotEqual, "!=", Location(begin, Position(m_line, m_column))); }
                break;
            case '<':
                if (Current() == '=') { Consume(); emit(TokenType::LessEqual, "<=", Location(begin, Position(m_line, m_column))); }
                else emit(TokenType::Less, "<", Location(begin, Position(m_line, m_column)));
                break;
            case '>':
                if (Current() == '=') { Consume(); emit(TokenType::GreaterEqual, ">=", Location(begin, Position(m_line, m_column))); }
                else emit(TokenType::Greater, ">", Location(begin, Position(m_line, m_column)));
                break;
            case '%': emit(TokenType::Modulo, "%", Location(begin, Position(m_line, m_column))); break;
            case '(': emit(TokenType::LeftParen, "(", Location(begin, Position(m_line, m_column))); break;
            case ')': emit(TokenType::RightParen, ")", Location(begin, Position(m_line, m_column))); break;
            case '{': emit(TokenType::LeftBrace, "{", Location(begin, Position(m_line, m_column))); break;
            case '}': emit(TokenType::RightBrace, "}", Location(begin, Position(m_line, m_column))); break;
            case '[': emit(TokenType::LeftBracket, "[", Location(begin, Position(m_line, m_column))); break;
            case ']': emit(TokenType::RightBracket, "]", Location(begin, Position(m_line, m_column))); break;
            case ',': emit(TokenType::Comma, ",", Location(begin, Position(m_line, m_column))); break;
            case '.': emit(TokenType::Dot, ".", Location(begin, Position(m_line, m_column))); break;
            case ':': emit(TokenType::Colon, ":", Location(begin, Position(m_line, m_column))); break;
            case ';': emit(TokenType::Semicolon, ";", Location(begin, Position(m_line, m_column))); break;
            default:
                emit(TokenType::Unknown, std::string(1, c), Location(Position(m_line, m_column), Position(m_line, m_column)));
                break;
            }
        }

        emit(TokenType::Eof, "", Location(Position(m_line, m_column), Position(m_line, m_column)));
    }
}