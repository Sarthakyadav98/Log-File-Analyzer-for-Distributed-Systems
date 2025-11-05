#ifndef LOG_PREPROCESSOR_H
#define LOG_PREPROCESSOR_H

#include <string>
#include <vector>

struct LogEntry {
    std::string timestamp;
    std::string level;
    std::string ip;
    std::string message;
};

struct PreprocessStats {
    std::size_t totalFiles = 0;
    std::size_t totalLines = 0;
    std::size_t validLines = 0;
};

std::vector<LogEntry> preprocessLogFile(const std::string &inputPath, std::size_t &totalLinesOut, std::size_t &validLinesOut);
void writeProcessedLogs(const std::vector<LogEntry> &logs, const std::string &outputPath);
PreprocessStats preprocessAllLogs(const std::string &inputDir, const std::string &outputDir);

#endif
