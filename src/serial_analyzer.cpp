// Purpose: Implements the serial (baseline) version of the log analyzer.
// Uses:

// log_parser.h → to read log lines.

// analyzer_utils.h → to store and print analysis results.

// timer.h → to measure runtime

#include "../include/log_parser.h"
#include "../include/analyzer_utils.h"
#include "../include/timer.h"
#include <iostream>
using namespace std;

int main() {
    vector<string> files = {"data/log1.txt", "data/log2.txt"};
    vector<string> logs = readMultipleLogs(files);

    Timer t;
    auto result = analyzeLogsSerial(logs);
    double time = t.elapsed();

    cout << "\n=== SERIAL ANALYSIS ===\n";
    displayResults(result);
    cout << "\nExecution Time: " << time << " seconds\n";
    return 0;
}
