
#include "urcl_optimizer.hpp"

#include <sstream>
#include <unordered_set>

static std::unordered_set<std::string> g_binops = {
    "add", "sub", "mlt", "div", "mod",
    "setl", "setg", "setge", "setle", "sete", "setne"
};

bool IsBinop(const std::string& value) 
{
    return g_binops.count(value);
}

// Source Functions
bool URCLOptimizer::NotEnd() 
{
    return m_index < m_source.size();
}

void URCLOptimizer::Advance(int i)
{
    m_index += i;
}

std::vector<std::string>& URCLOptimizer::EatInstruction()
{
    if (!NotEnd()) 
        return m_dummy;
    return m_source[m_index++];
}

std::vector<std::string>& URCLOptimizer::RequestInstruction(int i)
{
    if (m_index + i >= m_source.size())
        return m_dummy;
    return m_source[m_index+i];
}

// Initial Functions
void URCLOptimizer::SliceAssembly(const std::string& assembly)
{
    std::stringstream ss(assembly);
    std::string line;

    while (std::getline(ss, line)) {
        std::istringstream lineStream(line);
        std::string word;
        std::vector<std::string> tokens;

        while (lineStream >> word) {
            tokens.push_back(word);
        }
        if (!tokens.empty()) {
            m_source.push_back(tokens);
        }
    }
}

// Optimizer Functions
void URCLOptimizer::OutputEatInstruction()
{
    m_output.push_back(EatInstruction());
}

void URCLOptimizer::OutputPush(const std::vector<std::string>& ops)
{
    m_output.push_back(ops);
}

void URCLOptimizer::Skip()
{
    m_optimized = true;
    OutputEatInstruction();
}

void URCLOptimizer::CheckInstruction()
{
    if (m_index + 1 >= m_source.size()) {
        OutputEatInstruction();
        return;
    }

    auto& first = RequestInstruction(0);
    auto& second = RequestInstruction(1);
    auto& third = RequestInstruction(2);

    // Ugly peephole optimizations here, ill make this readable in the future..
    if (IsBinop(first[0]) && second[0] == "mov") {
        m_optimized = false;
        bool sameReg = first[1] == second[2];

        if (sameReg) {
            OutputPush({first[0], second[1], first[2], first[3]});
            Advance(2);
        } else {
            Skip();
        }
    } else if (first[0] == "imm" && second[0] == "brz") {
        m_optimized = false;
        bool isZero = first[2] == "0";
        bool sameReg = first[1] == second[2];

        if (isZero && sameReg) {
            OutputPush({"jmp", second[1]});
            Advance(2);
        } else {
            Skip();
        }
    } else if (first[0] == "llod" && second[0] == "mov") {
        m_optimized = false;
        bool sameReg = first[1] == second[2];

        if (sameReg) {
            OutputPush({"llod", second[1], first[2], first[3]});
            Advance(2);
        } else {
            Skip();
        }
    } else if (first[0] == "lstr" && second[0] == "llod") {
        m_optimized = false;
        bool sameBase = first[1] == second[2];
        bool sameOffset = first[2] == second[3];
        bool sameReg = first[3] ==  second[1];

        if (sameBase && sameOffset && sameReg) {
            Advance(2);
            OutputPush(first);
        } else {
            Skip();
        }
    } else if (first[0] == "psh" && second[0] == "pop") {
        m_optimized = false;
        std::string psh_a = first[1];
        std::string pop_a = second[1];

        if (psh_a == pop_a && pop_a[0] == 'r') {
            Advance(2);
        } else if (pop_a[0] == 'r' && psh_a[0] != 'r') {
            OutputPush({"imm", pop_a, psh_a});
            Advance(2);
        }
    } else if (first[0] == "psh" && second[0] == "imm" && third[0] == "pop") {
        m_optimized = false;
        std::string psh_a = first[1];
        std::string pop_a = third[1];

        if (pop_a[0] == psh_a[0] && pop_a[0] == 'r') {
            OutputPush({"mov", pop_a, psh_a});
        }
        if (pop_a[0] != psh_a[0] && pop_a[0] == 'r') {
            OutputPush({"imm", pop_a, psh_a});
        }
        OutputPush(second);
        Advance(3);
    } else if (first[0] == "psh" &&
         second[0] != "psh" &&
         second[0] != "pop" &&
         third[0]  == "pop")
    {
        m_optimized = false;
        OutputPush({"mov", third[1], first[1]});
        OutputPush(second);
        Advance(3);
    } else {
        OutputEatInstruction();
    }
}


std::string URCLOptimizer::RebuildOutput()
{
    std::string output;

    for (auto& out: m_output) {
        // Is Label
        if (out[0][0] == '.') {
            output += out[0];
        } else {
            output += "    ";
            for (std::string& ops: out) {
                output += ops + " ";
            }
        }
        output += '\n';
    }

    return output;
}

std::string URCLOptimizer::Optimize(const std::string& assembly)
{
    m_index = 0;
    m_optimized = false;
    m_source.clear(); 
    m_output.clear(); 
    SliceAssembly(assembly);

    do {
        m_index = 0;
        m_optimized = true;
        std::vector<std::vector<std::string>> next_source = std::move(m_output);
        m_output.clear();

        if (!next_source.empty()) {
            m_source = std::move(next_source);
        }

        while (NotEnd()) {
            CheckInstruction();
        }

    } while (!m_optimized);

    return RebuildOutput();
}