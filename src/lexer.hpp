
#pragma once

#include <vector>
#include <string>
#include <unordered_map>

enum class TokenType
{
    // Basic Types
    NUMBER,
    STRING,
    IDENT,
    CHAR,
    INVALID,
    END_OF_FILE,
    // Identifier Record Types
    RETURN,
    AUTO,
    EXTRN,
    ASM,
    IF,
    ELSE,
    WHILE,
    GOTO,
    // Independent Types
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    EQUAL,
    COMMA,
    AMPERSAND,
    SEMICOLON,
    OPENPAREN,
    CLOSEPAREN,
    OPENBRACE,
    CLOSEBRACE,
    COLON,
    QUESTION,
    ISGREATER,
    ISLESS,
    NOT,
    // Two Character Independent Types
    ISEQUAL,
    ISNEQUAL,
    ISGE,
    ISLE,
};

struct Token
{
    TokenType type;
    std::string value;
    size_t line, col;
};

static std::unordered_map<std::string, TokenType> _identRecord = {
    {"return", TokenType::RETURN},
    {"auto", TokenType::AUTO},
    {"extrn", TokenType::EXTRN},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"if", TokenType::IF},
    {"goto", TokenType::GOTO},
    {"__asm__", TokenType::ASM}
};

class Lexer 
{
public:
    std::vector<Token> Tokenize(const std::string& path);

private:
    // Lexer Info
    std::vector<Token> m_tokens;
    std::string m_source;
    size_t m_sourceIndex;
    size_t m_line;

    // Source Functions
    void Advance();
    char At() const;
    char Eat();
    char Next(int i = 1);
    bool NotEnd();

    // Push Function
    void PushToken(TokenType type, const std::string& value);

    // Lexer Functions
    void LexString();
    void LexNumber();
    void LexIdent();
    void LexOperand();
    void LexComment();
};