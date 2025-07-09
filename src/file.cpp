
#include "file.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

std::string File::ReadEverything(const std::string& path)
{
    std::ifstream file(path);
    if (!file) {
        std::cerr << "[FATAL ERROR]: could not find file '" << path << "'\n";
        std::cerr << "Compilation Terminated.";
        file.close();
        exit(1);
    }
    std::stringstream stream; 
    stream << file.rdbuf();
    return stream.str();
}