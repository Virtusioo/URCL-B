
#pragma once

#include "lexer.hpp"

#include <variant>
#include <sstream>
#include <optional>
#include <unordered_set>

enum class IRType
{
    LOAD_NUMBER,
    LOAD_FROMBASE,
    LOAD_GLOBAL,
    LOAD_STRING,
    ASSIGN_GLOBAL,
    DEREF,
    ASSIGN_FROMBASE,
    ASSIGN_MEMORY,
    INLINE_ASM,
    REF_FROMBASE,
    REF_GLOBAL,
    CALL,
    CALL_FUNCTION,
    BEGIN_WHILE,
    END_WHILE,
    END_WHILE_COND,
    BEGIN_IF,
    ADD_ELSE,
    END_IF,
    LOAD_RETURNED,
    RESERVE_STACK,
    PUT_LABEL,
    GOTO_LABEL,
    RETURN,
    RETURN_VALUE,
    BEGIN_TERNARY,
    GOTO_TERNARYEND,
    END_TERNARY,
    TERNARY_FALSE,
    GREATER,
    LESS,
    LE,
    GE,
    EQUAL,
    NEQUAL,
    NOT,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
};

enum class IRValuesType
{
    FUNCTION,
    ASM_FUNCTION,
    VARIABLE
};

using IRValue  = std::variant<IRType, std::string, std::vector<std::string>>;
using IRLocalInfo = std::unordered_map<std::string, size_t>;

struct IRValues
{
    IRValuesType type;
    std::vector<IRValue> values;
};

enum {
    IR_TYPE,
    IR_STRING,
    IR_STRINGLIST
};

struct IRGlobalInfo
{
    IRValues irValues;
    std::unordered_set<std::string> references;
};

struct IRInfo
{
    std::unordered_map<std::string, IRGlobalInfo> globalsMap;
    std::unordered_set<std::string> globals;
    std::unordered_set<std::string> references;
    std::unordered_set<std::string> strings;
};

class IRGenerator
{
public:
    IRInfo Generate(const std::vector<Token>& tokens);
    bool PrintErrors();
    void SetSourceName(const std::string& name);

private:
    // IR Info
    IRInfo m_irInfo;
    std::vector<Token> m_tokens;
    size_t m_tokenIndex;
    std::stringstream m_errors;
    bool m_gotError;
    std::string m_sourceName;
    std::string m_currentGlobal;
    std::unordered_map<std::string, int> m_params;
    std::vector<IRLocalInfo> m_locals;
    std::vector<size_t> m_localsSizeStack;
    size_t m_localsSize;
    size_t m_functionDepth;
    size_t m_localTotal;

    // Source Functions
    Token& At();
    Token& Eat();
    TokenType Type();
    void Advance();
    Token& Expect(TokenType type, const std::string& messsage);
    Token& Next(int i = 1);
    IRType EatOperand();
    void ExpectSemicolon();
    void SkipSemicolons();

    // Emit Functions
    void AddGlobal(const std::string& global, IRValuesType type);
    void Emit(std::initializer_list<IRValue> value);
    void Emit(const IRValue& value);

    // Character Parsers
    unsigned char ParseEscape(const std::string& string, size_t& i);
    std::string UnescapeString(const std::string& string);

    // Error Functions
    void Error(const std::string& message);

    // Asm Functions
    std::vector<std::string> GetAssembly();

    // IR Stack Related
    std::optional<int> GetLocal(const std::string& name, bool autoExtern = true);
    void PushLocal(const std::string& name);
    void PushBlock();
    void PopBlock();

    // IR Getters
    IRValues& GetIRValues();
    IRValues& GetIRValues(const std::string& global);
    IRGlobalInfo& GetIRGlobal();
    IRGlobalInfo& GetIRGlobal(const std::string& global);

    // IR Expr Functions
    void GenPrimary();
    void GenCall();
    void GenUnary();
    void GenMult();
    void GenAdd();
    void GenRelational();
    void GenEquality();
    void GenTernary();
    void GenParen();

    // IR Stmt Functions
    void GenDecl();
    void GenFunction();
    void GenBlock();
    void GenVarDecl();
    void GenDerefDecl();
    void GenExtrn();
    void GenAuto();
    void GenAsm();
    void GenAsmFunction();
    void GenReturn();
    void GenWhile();
    void GenGoto();
    void GenIf();

    // IR Main Functions
    void GenExpr();
    void GenStmt();
};