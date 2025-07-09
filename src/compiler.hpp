
#pragma once

#include "ir.hpp"

class Compiler
{
public:
    void LinkAndCompile(const std::vector<IRInfo>& irInfoList, const std::string& outputPath, bool optimize);

private:
    // Compiler Info
    std::vector<IRInfo> m_irInfoList;
    std::stringstream m_errors;
    bool m_gotError;
    std::stringstream m_data;
    std::stringstream m_output;
    std::unordered_map<std::string, std::string> m_strings;
    std::unordered_set<std::string> m_references;
    std::vector<std::string> m_whileStack;
    std::vector<std::string> m_ifStack;
    std::vector<std::string> m_ternaryStack;
    size_t m_heapBase;
    size_t m_labels;
    std::string m_leaveLabel;
    bool m_leaveLabelWasUsed;

    // Emitter Functions
    void Emit(const std::string& string);
    void EmitBasic(const std::string& string);

    // Compiler Functions
    IRValues GetGlobalValues(const std::string& name);
    void ResolveSymbols();
    void CompileStrings();
    void CompileEverything();
    void CompileFunction(const std::string& name, const std::vector<IRValue>& values);
    void CompileAsmFunction(const std::string& name, const std::vector<IRValue>& values);
    void CompileValues(const std::vector<IRValue>& value);
    void MakeBinop(const std::string& op);
    std::string MakeLabel();
    std::string GetLeave();
};