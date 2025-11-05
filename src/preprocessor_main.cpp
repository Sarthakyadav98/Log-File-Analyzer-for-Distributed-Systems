// Entry point for the log preprocessor tool
#include "../include/log_preprocessor.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: bin/preprocessor <input_dir> <output_dir>\n";
        return 1;
    }

    const std::string inputDir = argv[1];
    const std::string outputDir = argv[2];

    try {
        preprocessAllLogs(inputDir, outputDir);
    } catch (const std::exception &ex) {
        std::cerr << "[ERROR] Preprocessing failed: " << ex.what() << "\n";
        return 2;
    }

    return 0;
}
