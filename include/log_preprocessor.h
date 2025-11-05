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

std::vector<LogEntry> preprocessLogFile(const std::string &inputPath);
void writeProcessedLogs(const std::vector<LogEntry> &logs, const std::string &outputPath);
void preprocessAllLogs(const std::string &inputDir, const std::string &outputDir);

#endif
