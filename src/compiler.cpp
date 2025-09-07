
#include "compiler.hpp"
#include "urcl_optimizer.hpp"

#include <iostream>
#include <fstream>
#include <chrono>


std::string Compiler::MakeLabel()
{
    return ".L" + std::to_string(m_labels++) + "_";
}

void Compiler::ResolveSymbols()
{
    std::unordered_set<std::string> globals;
    m_references = {"main"};


    for (auto& irInfo: m_irInfoList) {
        for (const auto& global: irInfo.globals) {
            auto result = globals.insert(global);
            if (!result.second) {
                std::cerr << "[LINKER ERROR]: multiple definitions of symbol: '" << global << "'\n";
                m_gotError = true;
            }
        }
        m_references.insert(irInfo.references.begin(), irInfo.references.end());
    }

    for (const auto& symbol: m_references) {
        if (!globals.count(symbol)) {
            std::cerr << "[LINKER ERROR]: undefined symbol: '" << symbol << "'\n";
            m_gotError = true;
        }
    }

    // Dead Code Elimination
    for (auto& irInfo: m_irInfoList) {
        for (const auto& global: irInfo.globals) {
            auto& references = irInfo.globalsMap[global].references;

            // if global is not inside references, erase every symbol that the global needs
            // else, insert them again if needed
            if (!m_references.count(global)) {
                for (auto& symbol: references) {
                    irInfo.references.erase(symbol);
                }
            } else {
                for (auto& symbol: references) {
                    if (!m_references.count(symbol))
                        irInfo.references.insert(symbol);
                }  
            }

        }
    }
}

void Compiler::Emit(const std::string& basic) 
{
    m_output << "    " << basic << '\n';
}

void Compiler::EmitBasic(const std::string& basic)
{
    m_output << basic << '\n';
}

void Compiler::CompileStrings()
{
    std::unordered_set<std::string> strings;

    for (auto& irInfo: m_irInfoList) {
        for (auto& string: irInfo.strings) {
            strings.insert(string);
        }
    }
    for (auto& string: strings) {
        m_data << "    dw [";
        for (unsigned char c: string) {
            m_data << (int)c << ",";
        }
        m_data << 0;
        m_data << ']' << '\n';
        m_strings[string] = std::to_string(m_heapBase);
        // include '\0'
        m_heapBase += string.size() + 1;
    }
}

IRValues Compiler::GetGlobalValues(const std::string& name)
{
    for (auto& irInfo: m_irInfoList) {
        if (irInfo.globalsMap.find(name) != irInfo.globalsMap.end()) {
            return irInfo.globalsMap[name].irValues;
        }
    }
    return (IRValues){};
}

// Reserved for Compiler::CompileValues()
#define FetchOpcode() std::get<IR_TYPE>(values[i++])
#define FetchString() std::get<IR_STRING>(values[i++])
#define FetchStringList() std::get<IR_STRINGLIST>(values[i++])

void Compiler::CompileValues(const std::vector<IRValue>& values)
{
    size_t i = 0;
    std::string tmp, tmp2;
    size_t irSize = values.size();
    
    while (i < irSize) {
        auto& op = FetchOpcode();

        switch (op) {
            case IRType::INLINE_ASM: {
                auto& list = FetchStringList();
                for (auto str: list) {
                    Emit(str);
                }
                break;
            }
            case IRType::LOAD_NUMBER:
                tmp = FetchString();
                Emit("psh "+tmp);
                break;
            case IRType::LOAD_STRING:
                tmp = FetchString();
                Emit("psh "+m_strings[tmp]);
                break;
            case IRType::LOAD_FROMBASE:
                tmp = FetchString();
                Emit("llod r1 bp "+tmp);
                Emit("psh r1");
                break;
            case IRType::ASSIGN_FROMBASE:
                tmp = FetchString();
                Emit("pop r1");
                Emit("lstr bp "+tmp+" r1");
                break;
            case IRType::ASSIGN_MEMORY:
                Emit("pop r1");
                Emit("pop r2");
                Emit("str r2 r1");
                break;
            case IRType::REF_FROMBASE:
                tmp = FetchString();
                Emit("add r1 bp "+tmp);
                Emit("psh r1");
                break;
            case IRType::LOAD_GLOBAL: {
                tmp = FetchString();
                auto global = GetGlobalValues(tmp);
                if (global.type == IRValuesType::FUNCTION || global.type == IRValuesType::ASM_FUNCTION) {
                    Emit("psh ."+tmp);
                } else {
                    std::cerr << "unsupported global type: " << (int)global.type << '\n';
                    exit(1);
                }
                break;
            }
            case IRType::CALL: 
                tmp = FetchString();
                tmp2 = std::to_string(std::stoull(tmp) + 1);
                Emit("llod r1 sp "+tmp);
                Emit("cal r1");
                Emit("add sp sp "+tmp2);
                break;
            case IRType::CALL_FUNCTION: 
                tmp = FetchString();
                tmp2 = FetchString();
                Emit("cal ."+tmp);
                if (tmp2 != "0") {
                    Emit("add sp sp "+tmp2);
                }
                break;
            case IRType::LOAD_RETURNED:
                Emit("psh r1");
                break;
            case IRType::RETURN: {
                bool isLast = (i >= irSize);
                if (!isLast) {
                    Emit("jmp " + GetLeave());
                }
                break;
            }
            case IRType::RETURN_VALUE: {
                Emit("pop r1");
                bool isLast = (i >= irSize);
                if (!isLast) {
                    Emit("jmp " + GetLeave());
                }
                break;
            }
            case IRType::DEREF:
                Emit("lod r1 sp");
                Emit("lod r1 r1");
                Emit("str sp r1");
                break;
            case IRType::BEGIN_TERNARY:
                tmp = MakeLabel(); // false
                tmp2 = MakeLabel(); // true
                m_ternaryStack.push_back(tmp2);
                m_ternaryStack.push_back(tmp);
                Emit("pop r1");
                Emit("brz "+tmp+" r1");
                break;
            case IRType::GOTO_TERNARYEND:
                tmp = m_ternaryStack[m_ternaryStack.size()-2];
                Emit("jmp "+tmp);
                break;
            case IRType::TERNARY_FALSE:
                tmp = m_ternaryStack.back();
                m_ternaryStack.pop_back();
                EmitBasic(tmp);
                break;
            case IRType::END_TERNARY:
                tmp = m_ternaryStack.back();
                m_ternaryStack.pop_back();
                EmitBasic(tmp);
                break;        
            case IRType::BEGIN_IF:
                tmp = MakeLabel();
                Emit("pop r1");
                Emit("brz "+tmp+" r1");
                m_ifStack.push_back(tmp);
                break;
            case IRType::ADD_ELSE:
                tmp = m_ifStack.back();
                tmp2 = MakeLabel();
                m_ifStack.pop_back();
                Emit("jmp "+tmp2);
                EmitBasic(tmp);        
                m_ifStack.push_back(tmp2);
                break;
            case IRType::END_IF:
                tmp = m_ifStack.back();
                m_ifStack.pop_back();
                EmitBasic(tmp);
                break;
            case IRType::RESERVE_STACK:
                tmp = FetchString();
                Emit("sub sp sp "+tmp);
                break;
            case IRType::PUT_LABEL:
                tmp = FetchString();
                Emit("."+tmp);
                break;
            case IRType::GOTO_LABEL:
                tmp = FetchString();
                Emit("jmp ."+tmp);
                break;
            case IRType::BEGIN_WHILE:
                tmp = MakeLabel(); // begin
                EmitBasic(tmp);
                m_whileStack.push_back(tmp);
                break;
            case IRType::END_WHILE_COND:
                tmp = MakeLabel(); // end
                Emit("pop r1");
                Emit("brz "+tmp+" r1");
                m_whileStack.push_back(tmp);
                break;
            case IRType::END_WHILE:
                tmp = m_whileStack.back(); // end
                m_whileStack.pop_back();
                tmp2 = m_whileStack.back(); // begin
                m_whileStack.pop_back();
                Emit("jmp "+tmp2);
                EmitBasic(tmp);
                break;
            case IRType::EQUAL:
                MakeBinop("sete");
                break;
            case IRType::NEQUAL:
                MakeBinop("setne");
                break;
            case IRType::GREATER:
                MakeBinop("setg");
                break;
            case IRType::LESS:
                MakeBinop("setl");
                break;
            case IRType::GE:
                MakeBinop("setge");
                break;
            case IRType::LE:
                MakeBinop("setle");
                break;
            case IRType::NOT:
                Emit("pop r1");
                Emit("not r1 r1");
                Emit("psh r1");
                break;
            case IRType::ADD:
                MakeBinop("add");
                break;
            case IRType::SUB:
                MakeBinop("sub");
                break;
            case IRType::MUL:
                MakeBinop("mlt");
                break;
            case IRType::DIV:
                MakeBinop("div");
                break;
            default:
                std::cerr << "unsupported opcode: " << (int)op << '\n';
                exit(1);
        }
    }
}

void Compiler::MakeBinop(const std::string& op)
{
    Emit("pop r1");
    Emit("pop r2");
    Emit(op+" r1 r2 r1");
    Emit("psh r1");
}

void Compiler::CompileFunction(const std::string& name, const std::vector<IRValue>& values) 
{
    m_leaveLabelWasUsed = false;
    m_leaveLabel = name;
    EmitBasic("."+name);
    Emit("psh bp");
    Emit("mov bp sp");
    CompileValues(values);
    if (m_leaveLabelWasUsed) {
        EmitBasic(GetLeave());
    }
    Emit("mov sp bp");
    Emit("pop bp");
    Emit("ret");
}

std::string Compiler::GetLeave()
{
    m_leaveLabelWasUsed = true;
    return ".LEAVE"+m_leaveLabel+"_";
}

void Compiler::CompileAsmFunction(const std::string& name, const std::vector<IRValue>& values)
{
    EmitBasic("."+name);
    for (auto& str: values) {
        Emit(std::get<std::string>(str));
    }
    Emit("ret");
}

void Compiler::CompileEverything()
{
    for (auto& irInfo: m_irInfoList) {
        for (auto& global: irInfo.globals) {

            auto& globalValue = irInfo.globalsMap[global];
            IRValues& irValues = globalValue.irValues;

            // Skip Unused Global
            if (!m_references.count(global)) 
                continue; 

            switch (irValues.type) {
                case IRValuesType::FUNCTION:
                    CompileFunction(global, irValues.values);
                    break;
                case IRValuesType::ASM_FUNCTION:
                    CompileAsmFunction(global, irValues.values);
                    break;
            }
        }
    }
}

void Compiler::LinkAndCompile(const std::vector<IRInfo>& irInfoList, const std::string& outputPath)
{
    m_irInfoList = irInfoList;
    m_gotError = false;
    m_heapBase = 0;
    m_labels = 0;

    ResolveSymbols();
    if (m_gotError)
        return;

    std::ofstream outputFile(outputPath);
    URCLOptimizer optimizer;

    m_data << "\n//setup:\n";
    m_data << "    BITS == 16\n";
    m_data << "    MINSTACK 8192\n";
    m_data << "    MINHEAP 8192\n";
    m_data << "    @define bp r20\n";
    
    m_data << "\n//data:\n";

    CompileStrings();

    m_data << "    imm r25 " << m_heapBase << " // heap base\n";

    m_data << "\n//runtime:\n";
    m_output << "    cal .main\n    hlt\n";

    CompileEverything();
    
    std::string output = m_output.str();

    outputFile << m_data.str();
    outputFile << optimizer.Optimize(output);
}