#pragma once
#include <unordered_map>
#include <string>
#include "Position.h"

namespace Spaghett
{
    enum class TokenType
    {
        Identifier,
        String,

        Increment,  // ++
        Decrement,  // --

        Equal,          // =
        EqualEqual,     // ==
        NotEqual,       // !=
        Less,           // 
        LessEqual,      // <=
        Greater,        // >
        GreaterEqual,   // >=

        Number,     // Double

        Plus,
        Minus,
        Multiply,
        Divide,
        Modulo,     // %

        PlusEqual,      // +=
        MinusEqual,     // -=
        MultiplyEqual,  // *=
        DivideEqual,    // /=

        LeftParen,      // (
        RightParen,     // )
        LeftBrace,      // {
        RightBrace,     // }
        LeftBracket,    // [
        RightBracket,   // ]

        Comma,          // ,
        Dot,            // .
        Colon,          // :
        Semicolon,      // ;
        Concat,

        ReservedLocal,
        ReservedIf,
        ReservedElse,
        ReservedElseIf,
        ReservedWhile,
        ReservedFor,
        ReservedReturn,
        ReservedTrue,
        ReservedFalse,
        ReservedNull,
        ReservedAnd,
        ReservedOr,
        ReservedNot,
        ReservedFunction,
        ReservedThen,
        ReservedDo,
        ReservedEnd,
        ReservedIn,
        ReservedBreak,
        ReservedContinue,

        Unknown,
        Eof
    };

    inline std::unordered_map<std::string, TokenType> reservedKeywords = {
    {"local",     TokenType::ReservedLocal},
    {"if",        TokenType::ReservedIf},
    {"else",      TokenType::ReservedElse},
    {"elseif", TokenType::ReservedElseIf},
    {"while",     TokenType::ReservedWhile},
    {"for",       TokenType::ReservedFor},
    {"break", TokenType::ReservedBreak},
    {"continue", TokenType::ReservedContinue},
    {"return",    TokenType::ReservedReturn},
    {"true",      TokenType::ReservedTrue},
    {"false",     TokenType::ReservedFalse},
    {"null",       TokenType::ReservedNull},
    {"and",       TokenType::ReservedAnd},
    {"or",        TokenType::ReservedOr},
    {"not",       TokenType::ReservedNot},
    {"func", TokenType::ReservedFunction},
    {"then" , TokenType::ReservedThen },
    {"do", TokenType::ReservedDo},
    {"end",       TokenType::ReservedEnd},
    {"in" , TokenType::ReservedIn}
    };

    struct TokenWithData
    {
        TokenType type;
        std::string value;
        Spaghett::Location location;
    };

    inline TokenType IsReservedKeyWord(std::string str)
    {
        auto it = reservedKeywords.find(str);

        if (it != reservedKeywords.end())
        {
            return it->second;
        }

        return TokenType::Unknown;
    }

    inline std::string TokenTypeToString(TokenType type)
    {
        switch (type)
        {
        case TokenType::Identifier:         return "Identifier";
        case TokenType::String:             return "String";
        case TokenType::Number:             return "Number";
        case TokenType::Increment:          return "Increment";
        case TokenType::ReservedElseIf: return "ReservedElseIf";
        case TokenType::Decrement:          return "Decrement";
        case TokenType::Equal:              return "Equal";
        case TokenType::EqualEqual:         return "EqualEqual";
        case TokenType::NotEqual:           return "NotEqual";
        case TokenType::Less:               return "Less";
        case TokenType::LessEqual:          return "LessEqual";
        case TokenType::Greater:            return "Greater";
        case TokenType::GreaterEqual:       return "GreaterEqual";
        case TokenType::Plus:               return "Plus";
        case TokenType::Minus:              return "Minus";
        case TokenType::Multiply:           return "Multiply";
        case TokenType::Divide:             return "Divide";
        case TokenType::Modulo:             return "Modulo";
        case TokenType::PlusEqual:          return "PlusEqual";
        case TokenType::MinusEqual:         return "MinusEqual";
        case TokenType::MultiplyEqual:      return "MultiplyEqual";
        case TokenType::DivideEqual:        return "DivideEqual";
        case TokenType::LeftParen:          return "LeftParen";
        case TokenType::RightParen:         return "RightParen";
        case TokenType::LeftBrace:          return "LeftBrace";
        case TokenType::RightBrace:         return "RightBrace";
        case TokenType::LeftBracket:        return "LeftBracket";
        case TokenType::RightBracket:       return "RightBracket";
        case TokenType::Comma:              return "Comma";
        case TokenType::Dot:                return "Dot";
        case TokenType::Colon:              return "Colon";
        case TokenType::Semicolon:          return "Semicolon";
        case TokenType::ReservedLocal:      return "ReservedLocal";
        case TokenType::ReservedIf:         return "ReservedIf";
        case TokenType::ReservedElse:       return "ReservedElse";
        case TokenType::ReservedWhile:      return "ReservedWhile";
        case TokenType::ReservedFor:        return "ReservedFor";
        case TokenType::ReservedReturn:     return "ReservedReturn";
        case TokenType::ReservedTrue:       return "ReservedTrue";
        case TokenType::ReservedFalse:      return "ReservedFalse";
        case TokenType::ReservedNull:        return "ReservedNull";
        case TokenType::ReservedAnd:        return "ReservedAnd";
        case TokenType::ReservedOr:         return "ReservedOr";
        case TokenType::ReservedNot:        return "ReservedNot";
        case TokenType::ReservedFunction:   return "ReservedFunction";
        case TokenType::ReservedEnd:        return "ReservedEnd";
        case TokenType::ReservedDo: return "ReservedDo";
        case TokenType::ReservedThen: return "ReservedThen";
        case TokenType::ReservedIn: return "ReservedIn";
        case TokenType::ReservedBreak: return "ReservedBreak";
        case TokenType::ReservedContinue: return "ReservedContinue";
        case TokenType::Unknown:            return "Unknown";
        case TokenType::Eof:                return "Eof";
        default:                            return "???";
        }
    }

}