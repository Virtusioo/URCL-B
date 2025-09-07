
#pragma once

#include <string>
#include <vector>

class URCLOptimizer 
{
public:
    std::string Optimize(const std::string& assembly);

private:
    std::vector<std::vector<std::string>> m_source;
    std::vector<std::vector<std::string>> m_output;
    std::vector<std::string> m_dummy = {"dummy"};
    size_t m_index;
    bool m_optimized;

    // Source Functions
    void Advance(int i = 1);
    bool NotEnd();
    std::vector<std::string>& EatInstruction();
    std::vector<std::string>& RequestInstruction(int i);

    // Initial Functions
    void SliceAssembly(const std::string& assembly);

    // Optimizer Functions
    void OutputPush(const std::vector<std::string>& ops);
    void OutputEatInstruction();
    void CheckInstruction();
    void Skip();
    std::string RebuildOutput();
};