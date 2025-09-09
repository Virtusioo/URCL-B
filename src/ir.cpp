
#include "ir.hpp"

#include <string>
#include <iomanip>
#include <iostream>

// Character Parsers
unsigned char IRGenerator::ParseEscape(const std::string& s, size_t& i) 
{
    if (s[i] != '\\') 
        return s[i];

    i += 1;

    if (i >= s.size()) {
        Error("incomplete escape sequence");
        return 0;
    }

    switch (s[i]) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '0': return '\0';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"': return '\"';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'v': return '\v';
        case 'a': return '\a';

        case 'x': { 
            if (i + 2 >= s.size()) {
                Error("incomplete hex escape");
                return 0;
            }
            std::string hex = s.substr(i + 1, 2);
            i += 2;
            return static_cast<char>(std::stoi(hex, nullptr, 16));
        }

        case 'u': { 
            if (i + 4 >= s.size()) {
                Error("incomplete unicode escape");
                return 0;
            }
            std::string hex = s.substr(i + 1, 4);
            i += 4;
            return static_cast<char>(std::stoi(hex, nullptr, 16)); // UTF-8 simplified
        }

        default:
            Error("unknown escape sequence: \\" + std::string(1, s[i]));
            return 0;
    }
}

std::string IRGenerator::UnescapeString(const std::string& input) 
{
    std::string output;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\\') {
            output += ParseEscape(input, i);
        } else {
            output += input[i];
        }
    }
    return output;
}

// Source Functions
Token& IRGenerator::At()
{
    return m_tokens[m_tokenIndex];
}

Token& IRGenerator::Eat()
{
    Token& t = At();
    Advance();
    return t;
}

TokenType IRGenerator::Type()
{
    return At().type;
}

void IRGenerator::Advance()
{
    if (Type() == TokenType::END_OF_FILE)
        return;
    m_tokenIndex += 1;
}

Token& IRGenerator::Next(int i)
{
    return m_tokens[(int)m_tokenIndex + i];
}

Token& IRGenerator::Expect(TokenType type, const std::string& messsage)
{
    Token& t = Eat();
    if (t.type != type) {
        Error(messsage+", got '"+t.value+"'");
    }
    return t;
}

void IRGenerator::SkipSemicolons()
{
    while (Type() == TokenType::SEMICOLON)
        Advance();
}

void IRGenerator::ExpectSemicolon()
{
    Expect(TokenType::SEMICOLON, "expected ';' after declaration");
}

// Reserved for IRGenerator::EatOperand()
#define CaseOperand(tokenType, irType) case tokenType: return irType

// I'm crying
IRType IRGenerator::EatOperand()
{
    Token& t = Eat();
    switch (t.type) {
        CaseOperand(TokenType::PLUS, IRType::ADD);
        CaseOperand(TokenType::MINUS, IRType::SUB);
        CaseOperand(TokenType::STAR, IRType::MUL);
        CaseOperand(TokenType::SLASH, IRType::DIV);
        CaseOperand(TokenType::PERCENT, IRType::MOD);
        CaseOperand(TokenType::ISGREATER, IRType::GREATER);
        CaseOperand(TokenType::ISLESS, IRType::LESS);
        CaseOperand(TokenType::ISGE, IRType::GE);
        CaseOperand(TokenType::ISLE, IRType::LE);
        CaseOperand(TokenType::ISEQUAL, IRType::EQUAL);
        CaseOperand(TokenType::ISNEQUAL, IRType::NEQUAL);
    }
    return (IRType)0;
}

// Emit Functions
void IRGenerator::AddGlobal(const std::string& label, IRValuesType type)
{
    if (m_irInfo.globals.count(label)) {
        Error("global '"+label+"' already exists");
    }
    IRValues values;
    values.type = type;

    m_irInfo.globals.insert(label);
    GetIRValues(label) = std::move(values);
    m_currentGlobal = label;
}
void IRGenerator::Emit(std::initializer_list<IRValue> value)
{
    for (IRValue v: value) {
        if (m_currentGlobal == "?") 
            continue;
        GetIRValues().values.emplace_back(v);
    }
}

void IRGenerator::Emit(const IRValue& value)
{
    if (m_currentGlobal == "?") 
        return;
    GetIRValues().values.emplace_back(value);
}

// IR Stack Related
std::optional<int> IRGenerator::GetLocal(const std::string& name, bool autoExtern)
{
    for (int i = (int)m_locals.size() - 1; i >= 0; --i) {
        auto& scope = m_locals[i];
        if (scope.find(name) != scope.end()) {
            return -scope[name];
        }
    }
    if (m_params.find(name) != m_params.end()) {
        return m_params[name];
    }
    if (m_currentGlobal != "?" && autoExtern) {
        m_irInfo.references.insert(name);
    }
    
    return {};
}


void IRGenerator::PushLocal(const std::string& name)
{
    auto& scope = m_locals.back();
    scope[name] = m_localsSize++;
    m_localTotal += 1;
}

void IRGenerator::PushBlock()
{
    m_localsSizeStack.push_back(m_localsSize);
    m_locals.emplace_back();
}

void IRGenerator::PopBlock()
{
    m_localsSize = m_localsSizeStack.back();
    m_locals.pop_back();
    m_localsSizeStack.pop_back();
}

// IR Getters
IRValues& IRGenerator::GetIRValues()
{
    return m_irInfo.globalsMap[m_currentGlobal].irValues;
}

IRValues& IRGenerator::GetIRValues(const std::string& name)
{
    return m_irInfo.globalsMap[name].irValues;
}

IRGlobalInfo& IRGenerator::GetIRGlobal(const std::string& name)
{
    return m_irInfo.globalsMap[name];
}

IRGlobalInfo& IRGenerator::GetIRGlobal()
{
    return m_irInfo.globalsMap[m_currentGlobal];
}

// IR  Expr Functions
void IRGenerator::GenPrimary()
{
    Token& t = Eat();

    if (m_currentGlobal == "?")
        return;

    switch (t.type) {
        case TokenType::NUMBER: {
            Emit({IRType::LOAD_NUMBER, t.value}); 
            break;
        }
        case TokenType::STRING: {
            std::string string = UnescapeString(t.value);
            m_irInfo.strings.insert(string); 
            Emit({IRType::LOAD_STRING, string});
            break;
        }
        case TokenType::CHAR: {
            size_t i = 0;
            Emit({IRType::LOAD_NUMBER, std::to_string(ParseEscape(t.value, i))});
            break;
        }
        case TokenType::IDENT: {
            auto local = GetLocal(t.value);

            if (local.has_value()) {
                Emit({IRType::LOAD_FROMBASE, std::to_string(local.value())});
                break;
            }

            if (m_irInfo.references.count(t.value)) {
                Emit({IRType::LOAD_GLOBAL, t.value});
                break;
            }
            if (m_irInfo.globals.count(t.value)) {
                Emit({IRType::LOAD_GLOBAL, t.value});
                GetIRGlobal(m_currentGlobal).references.insert(t.value);
                break;
            }

            break;
        }
        case TokenType::OPENPAREN: {
            GenExpr();
            Expect(TokenType::CLOSEPAREN, "expected ')' when closing expression");
            break;
        }
        default: {
            Error("unexpected symbol '"+t.value+"'");
            break;
        }
    }
}

void IRGenerator::GenCall()
{
    if (Next().type == TokenType::OPENPAREN) {
        Token& t = At();
        bool isGlobalFunction = false;
        bool isGlobalLocal = false;
        if (t.type == TokenType::IDENT) {
            auto local = GetLocal(t.value);

            isGlobalFunction = m_irInfo.references.count(t.value) && !local.has_value();
            isGlobalLocal = m_irInfo.globals.count(t.value);
        }
        if (!isGlobalFunction) {
            GenPrimary();
        } else {
            m_irInfo.references.insert(t.value);
            if (isGlobalLocal) {
                GetIRGlobal(t.value).references.insert(t.value);
            }
            Advance();
        }
        Advance();

        size_t count = 0;
        if (Type() != TokenType::CLOSEPAREN) {
            GenExpr();
            count += 1;
        }
        while (Type() != TokenType::CLOSEPAREN && Type() != TokenType::END_OF_FILE) {
            Expect(TokenType::COMMA, "expected ',' when seperating argument #"+std::to_string(count+1));
            GenExpr();
            count += 1;
        }
        Expect(TokenType::CLOSEPAREN, "expected ')' when closing argument list");

        if (isGlobalFunction) {
            Emit({IRType::CALL_FUNCTION, t.value, std::to_string(count)});
        } else {
            Emit({IRType::CALL, std::to_string(count)});
        }
        Emit(IRType::LOAD_RETURNED);
    } else {
        GenPrimary();
    }
}

void IRGenerator::GenUnary()
{
    if (Type() == TokenType::STAR) {
        while (Type() == TokenType::STAR) {
            Advance();
            GenCall();
            Emit(IRType::DEREF);
        } 
    } else if (Type() == TokenType::AMPERSAND) {
        Advance();
        Token& value = Expect(TokenType::IDENT, "expected lvalue next to address-of operator");
        if (value.type == TokenType::IDENT) {
            if (m_irInfo.globals.count(value.value)) {
                Emit({IRType::REF_GLOBAL, value.value});
                return;
            }
            auto local = GetLocal(value.value);
            if (local.has_value()) {
                Emit({IRType::REF_FROMBASE, std::to_string(local.value())});
            }
        }
    } else if (Type() == TokenType::NOT) {
        Advance();
        GenCall();
        Emit(IRType::NOT);
    } else {
        GenCall();
    }
}

void IRGenerator::GenMult()
{
    GenUnary();
    while (
        Type() == TokenType::STAR || 
        Type() == TokenType::SLASH || 
        Type() == TokenType::PERCENT
    ) {
        IRType op = EatOperand();
        GenUnary();
        Emit(op);
    }
}

void IRGenerator::GenAdd()
{
    GenMult();
    while (Type() == TokenType::PLUS || Type() == TokenType::MINUS) {
        IRType op = EatOperand();
        GenMult();
        Emit(op);
    } 
}

void IRGenerator::GenRelational()
{
    GenAdd();
    while (
        Type() == TokenType::ISGE || 
        Type() == TokenType::ISLE || 
        Type() == TokenType::ISGREATER || 
        Type() == TokenType::ISLESS
    ) {
        IRType op = EatOperand();
        GenAdd();
        Emit(op);
    }
}

void IRGenerator::GenEquality()
{
    GenRelational();
    while (Type() == TokenType::ISEQUAL || Type() == TokenType::ISNEQUAL) {
        IRType op = EatOperand();
        GenRelational();
        Emit(op);
    }
}

void IRGenerator::GenTernary()
{
    GenEquality();
    while (Type() == TokenType::QUESTION) {
        Emit(IRType::BEGIN_TERNARY);
        Advance();
        GenEquality();
        Emit(IRType::GOTO_TERNARYEND);
        Expect(TokenType::COLON, "expected ':' when seperating ternary expression");
        Emit(IRType::TERNARY_FALSE);
        GenEquality();
        Emit(IRType::END_TERNARY);
    }
}

void IRGenerator::GenParen()
{
    Expect(TokenType::OPENPAREN, "expected '(' when opening expression");
    GenExpr();
    Expect(TokenType::CLOSEPAREN, "expected ')' when closing expression");
}

// IR Stmt Functions
void IRGenerator::GenFunction() 
{
    if (m_functionDepth) {
        Error("cannot define function here");
    }
    const std::string& name = Eat().value;
    m_functionDepth += 1;

    AddGlobal(name, IRValuesType::FUNCTION);
    Advance();
    m_irInfo.globals.emplace(name);
    m_params.clear();


    size_t param = 1;
    std::vector<std::string> params;

    if (Type() != TokenType::CLOSEPAREN) {
        Token& t = Expect(TokenType::IDENT, "invalid parameter #1");
        if (t.type == TokenType::IDENT) 
            params.push_back(t.value);
        param += 1;
    }

    while (Type() != TokenType::CLOSEPAREN && Type() != TokenType::END_OF_FILE) {
        Expect(TokenType::COMMA, "expected ',' beside parameter #"+std::to_string(param-1));

        Token& t = Expect(TokenType::IDENT, "invalid parameter #"+std::to_string(param));
        if (t.type == TokenType::IDENT) 
            params.push_back(t.value);
        param += 1;
    }

    Expect(TokenType::CLOSEPAREN, "expected ')' when closing parameters");
    // if params != valid params, skip
    if (params.size() != param-1) 
        return;

    for (auto& p: params) {
        m_params[p] = param--;
    }

    GenStmt();
    m_params.clear();
    m_functionDepth -= 1;
    m_currentGlobal = "?";
}

void IRGenerator::GenBlock()
{
    if (!m_functionDepth) {
        Error("expected declaration next to block");
    }
    Expect(TokenType::OPENBRACE, "expected '{' when opening scope");
    PushBlock();
    while (Type() != TokenType::CLOSEBRACE && Type() != TokenType::END_OF_FILE) {
        GenStmt();
    }
    PopBlock();
    Expect(TokenType::CLOSEBRACE, "expected '}' when closing scope");
}

void IRGenerator::GenVarDecl()
{
    const std::string& var = Eat().value;
    auto local = GetLocal(var, false);
    Advance();
    if (!m_functionDepth) {
        GenPrimary();
        AddGlobal(var, IRValuesType::VARIABLE);
        m_currentGlobal = "?";
        return;
    }
    if (local.has_value()) {
        GenExpr();
        Emit({IRType::ASSIGN_FROMBASE, std::to_string(local.value())});
        return;
    } else {
        if (m_irInfo.globals.count(var)) {
            if (GetIRValues(var).type != IRValuesType::VARIABLE) {
                Error("cannot assign to a non global variable");
                return;
            }
            Emit({IRType::ASSIGN_GLOBAL, var});
            return;
        }
        GenExpr();
        PushLocal(var);
    }
}

void IRGenerator::GenAsmFunction()
{
    if (m_functionDepth) {
        Error("cannot define assembly function here");
    }
    m_functionDepth += 1;
    const std::string& name = Eat().value;
    Advance();
    AddGlobal(name, IRValuesType::ASM_FUNCTION);
    auto& block = GetIRValues().values;
    auto assembly = GetAssembly();
    for (auto& str: assembly) {
        block.push_back(str);
    }
    m_functionDepth -= 1;
    m_currentGlobal = "?";
}

void IRGenerator::GenDecl()
{
    const Token& next = Next();

    if (next.type == TokenType::OPENPAREN) {
        if (m_functionDepth) {
            GenCall();
            ExpectSemicolon();
            GetIRValues().values.pop_back(); // pop LOAD_RETURNED
            return;
        }
        GenFunction();
    } else if (next.type == TokenType::EQUAL) {
        GenVarDecl();
        ExpectSemicolon();
    } else if (next.type == TokenType::ASM) {
        GenAsmFunction();
    } else if (next.type == TokenType::COLON) {
        Emit({IRType::PUT_LABEL, Eat().value});
        Advance();
    } else {
        Advance();
        Error("expected declaration");
    }
}

void IRGenerator::GenDerefDecl()
{
    Advance();
    GenExpr();
    Expect(TokenType::EQUAL, "expected '=' as assignment");
    GenExpr();
    Emit(IRType::ASSIGN_MEMORY);
    ExpectSemicolon();
}

void IRGenerator::GenExtrn()
{
    Advance();
    Token t = Expect(TokenType::IDENT, "expected identifier when externing");

    m_irInfo.references.insert(t.value);

    while (Type() == TokenType::COMMA) {
        Expect(TokenType::COMMA, "expected ',' when seperating externed");
        t = Expect(TokenType::IDENT, "expected identifier when externing");
        m_irInfo.references.insert(t.value);
    }
    ExpectSemicolon();
}

void IRGenerator::GenAuto()
{
    Advance();
    if (!m_functionDepth) {
        Error("cannot declare auto variable here");
        return;
    }
    Token t = Expect(TokenType::IDENT, "expected identifier when declaring auto");
    PushLocal(t.value);
    size_t i = 1;
    while (Type() == TokenType::COMMA) {
        Advance();
        t = Expect(TokenType::IDENT, "expected identifier when declaring auto");
        PushLocal(t.value);
        i += 1;
    }
    Emit({IRType::RESERVE_STACK, std::to_string(i)});
    ExpectSemicolon();
}

void IRGenerator::GenAsm()
{
    Advance();
    Emit({IRType::INLINE_ASM, GetAssembly()});
}

void IRGenerator::GenReturn()
{
    if (!m_functionDepth) {
        Error("return statement cannot be here");
    }
    Advance();
    if (Type() == TokenType::SEMICOLON) {
        Emit(IRType::RETURN);
    } else {
        GenExpr();
        Emit(IRType::RETURN_VALUE);
    }
    ExpectSemicolon();
}

void IRGenerator::GenWhile()
{
    if (!m_functionDepth) {
        Error("cannot declare while loop here");
    }
    Advance();
    Emit(IRType::BEGIN_WHILE);
    GenParen();
    Emit(IRType::END_WHILE_COND);
    GenStmt();
    Emit(IRType::END_WHILE);
}

void IRGenerator::GenGoto()
{
    Advance();
    Emit({IRType::GOTO_LABEL, Eat().value});
    ExpectSemicolon();
}

void IRGenerator::GenIf()
{
    Advance();
    GenParen();
    Emit(IRType::BEGIN_IF);
    GenStmt();
    if (Type() == TokenType::ELSE) {
        Advance();
        Emit(IRType::ADD_ELSE);
        GenStmt();
    }
    Emit(IRType::END_IF);
}

std::vector<std::string> IRGenerator::GetAssembly()
{
    std::vector<std::string> assembly;
    Expect(TokenType::OPENBRACE, "expected '{' when starting assembly block");
    while (Type() != TokenType::CLOSEBRACE && Type() != TokenType::END_OF_FILE) {
        assembly.push_back(Expect(TokenType::STRING, "expected string literal in assembly block").value);
    }
    Expect(TokenType::CLOSEBRACE, "expected '}' when closing assembly block");
    return assembly;
}

// IR Main Functions
void IRGenerator::GenExpr()
{
    GenTernary();
}

void IRGenerator::GenStmt()
{
    switch (Type()) {
        case TokenType::IDENT: 
            GenDecl(); break;
        case TokenType::STAR:
            GenDerefDecl(); break;
        case TokenType::EXTRN:
            GenExtrn(); break;
        case TokenType::OPENBRACE:
            GenBlock(); break;
        case TokenType::AUTO:
            GenAuto(); break;
        case TokenType::ASM:
            GenAsm(); break;
        case TokenType::RETURN:
            GenReturn(); break;
        case TokenType::WHILE:
            GenWhile(); break;
        case TokenType::GOTO:
            GenGoto(); break;
        case TokenType::IF:
            GenIf(); break;
        default:
            Error("expected declaration");
            GenExpr(); 
            break;
    }
    SkipSemicolons();
}

// Error Stuff
void IRGenerator::Error(const std::string& message) 
{
    Token& t = m_tokenIndex == 0 ? At() : Next(-1);
    m_gotError = true;
    m_errors << "[SYNTAX ERROR]: " << m_sourceName << ':' << t.line << ": " << m_currentGlobal << ": " << message << '\n';
}

bool IRGenerator::PrintErrors()
{
    if (m_gotError)
        std::cerr << m_errors.str();
    return m_gotError;
}

void IRGenerator::SetSourceName(const std::string& name)
{
    m_sourceName = name;
}

// Main
IRInfo IRGenerator::Generate(const std::vector<Token>& tokens)
{
    m_tokens = tokens;
    m_tokenIndex = 0;
    m_errors.clear();
    m_gotError = false;
    m_currentGlobal = "?";
    m_params.clear();
    m_locals.clear();
    m_localsSizeStack.clear();
    m_localsSize = 1;
    m_functionDepth = 0;

    while (Type() != TokenType::END_OF_FILE) {
        GenStmt();
    }

    return std::move(m_irInfo);
}