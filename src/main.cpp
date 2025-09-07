
#include "lexer.hpp"
#include "ir.hpp"
#include "compiler.hpp"

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

static inline int PrintUsage()
{
    std::cout << "[USAGE]:\n    bcc <...input> -o <output>";
    return 1;
}

int main(int argc, char** argv)
{
    if (argc == 1)
        return PrintUsage();

    bool nostdlib = false;
    std::vector<std::string> inputFiles;
    std::string outputFile;

    // ill make this cleaner in the future
    for (int i = 1; i < argc; ++i) {
        std::string str = argv[i];
        if (str == "-nostdlib") {
            nostdlib = true;
        } else if (str == "-o") {
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                std::cerr << "[CLI ERROR]: No output file specified after -o!\n";
                return PrintUsage();
            }
            outputFile = argv[++i];
            fs::path path = outputFile;
            if (path.extension().empty()) {
                outputFile += ".urcl";
            }
        } else {
            inputFiles.push_back(str);
        }
    }

    if (outputFile.empty()) {
        std::cerr << "[FATAL ERROR]: No output file provided!\n";
        return PrintUsage();
    }

    if (inputFiles.empty()) {
        std::cerr << "[FATAL ERROR]: No input files provided!\n";
        return PrintUsage();
    }

    Lexer lexer;
    IRGenerator irGen;
    Compiler compiler;

    std::vector<IRInfo> toLink;

    // Library Code
    if (!nostdlib) {
        try {
            for (auto& entry: fs::recursive_directory_iterator("lib")) {
                if (fs::is_regular_file(entry) && entry.path().extension() == ".b") {
                    std::string source = entry.path().string();

                    irGen.SetSourceName(source);
                    auto tokens = lexer.Tokenize(source);
                    auto irInfo = irGen.Generate(tokens);
                    if (irGen.PrintErrors())
                        return 1;
                    toLink.push_back(irInfo);
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "[FATAL ERROR]: " << e.what() << '\n';
            std::cerr << "Compilation Terminated.";
            return 1;
        }
    }

    // User Code
    for (auto& source: inputFiles) {

        irGen.SetSourceName(source);
        auto tokens = lexer.Tokenize(source);
        auto irInfo = irGen.Generate(tokens);
        if (irGen.PrintErrors())
            return 1;
        toLink.push_back(irInfo);
    }

    compiler.LinkAndCompile(toLink, outputFile);
}