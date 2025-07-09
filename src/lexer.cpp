
#include "lexer.hpp"
#include "file.hpp"

// Source Functions
void Lexer::Advance()
{
    if (At() == '\n') {
        m_line += 1;
    }
    m_sourceIndex += 1;
}

char Lexer::At() const
{
    return m_source[m_sourceIndex];
}

char Lexer::Eat()
{
    char c = At();
    Advance();
    return c;
}

char Lexer::Next(int i) 
{
    return m_source[m_sourceIndex + i];
}

bool Lexer::NotEnd()
{
    return m_sourceIndex < m_source.size();
}

// Push Function
void Lexer::PushToken(TokenType type, const std::string& value)
{
    m_tokens.emplace_back(Token{type, value, m_line});
}

// Lexer Functions
void Lexer::LexString()
{
    char quote = Eat();
    TokenType type = quote == '\'' ? TokenType::CHAR : TokenType::STRING;

    std::string data;
    char c;

    while ((c = At()) != quote && c != '\0') {
        data += Eat();
    }
    if (Eat() == '\0') {
        PushToken(TokenType::INVALID, "end of file");
        return;
    }
    PushToken(type, data);
}

#include <sstream>
#include <iomanip>

void Lexer::LexNumber()
{
    std::string raw;
    int value = 0;

    if (At() == '0' && (Next() == 'x' || Next() == 'X')) {
        Advance(); 
        Advance(); 

        while (isxdigit(At())) {
            raw += Eat();
        }

        std::stringstream ss;
        ss << std::hex << raw;
        ss >> value;

        PushToken(TokenType::NUMBER, std::to_string(value));
        return;
    }

    while (isdigit(At())) {
        raw += Eat();
    }

    PushToken(TokenType::NUMBER, raw);
}


void Lexer::LexIdent()
{
    TokenType type = TokenType::IDENT;
    std::string ident(1, Eat());
    char c;

    while (isalpha(c = At()) || isdigit(c) || c == '_') {
        ident += Eat();
    }

    if (_identRecord.find(ident) != _identRecord.end()) 
        type = _identRecord[ident];
    
    PushToken(type, ident);
}

// Reserved for Lexer::LexOperand()
#define CaseOperand(operandChar, tokenType) \
    case operandChar: PushToken(tokenType, operand); return;

#define WhenOperandAt(operandChar, tokenType) \
    if (At() == operandChar) { \
        operand += Eat(); \
        PushToken(tokenType, operand); \
    } else

void Lexer::LexOperand()
{
    char c = Eat();
    std::string operand(1, c);

    switch (c) {
        CaseOperand('+', TokenType::PLUS);
        CaseOperand('-', TokenType::MINUS);
        CaseOperand('*', TokenType::STAR);
        CaseOperand('/', TokenType::SLASH);
        CaseOperand('{', TokenType::OPENBRACE);
        CaseOperand('}', TokenType::CLOSEBRACE);
        CaseOperand('(', TokenType::OPENPAREN);
        CaseOperand(')', TokenType::CLOSEPAREN);
        CaseOperand(',', TokenType::COMMA);
        CaseOperand('&', TokenType::AMPERSAND);
        CaseOperand(';', TokenType::SEMICOLON);
        CaseOperand(':', TokenType::COLON);
        CaseOperand('?', TokenType::QUESTION);
        case '!': {
            WhenOperandAt('=', TokenType::ISNEQUAL)
            PushToken(TokenType::NOT, operand);
            return;
        }
        case '<': {
            WhenOperandAt('=', TokenType::ISLE)
            PushToken(TokenType::ISLESS, operand);
            return;
        }
        case '>': {
            WhenOperandAt('=', TokenType::ISGE)
            PushToken(TokenType::ISGREATER, operand);
            return;
        }
        case '=': {
            WhenOperandAt('=', TokenType::ISEQUAL)
            PushToken(TokenType::EQUAL, operand);
            return;
        }
    }
    PushToken(TokenType::INVALID, operand);
}

void Lexer::LexComment()
{
    m_sourceIndex += 2;
    while (NotEnd()) {
        if (At() == '*' && Next() == '/') {
            m_sourceIndex += 2;
            return; // already finished comment
        }
        Advance();
    }
    PushToken(TokenType::INVALID, "unterminated comment");
}

std::vector<Token> Lexer::Tokenize(const std::string& path) 
{
    m_source = File::ReadEverything(path);
    m_line = 1;
    m_sourceIndex = 0;

    while (NotEnd()) {
        char c = At();
        switch (c) {
            case '\r':
            case ' ':
            case '\n':
                Advance();
                break;
            case '\'':
            case '"':
                LexString();
                break;
            case '/':
                if (Next() == '*') {
                    LexComment();
                    break;
                }
            default: {
                if (isalpha(c) || c == '_') {
                    LexIdent();
                    break;
                } 
                if (isdigit(c)) {
                    LexNumber();
                    break;
                }
                LexOperand();
                break;
            }
        }
    }

    PushToken(TokenType::END_OF_FILE, "end of file");
    return std::move(m_tokens);
}