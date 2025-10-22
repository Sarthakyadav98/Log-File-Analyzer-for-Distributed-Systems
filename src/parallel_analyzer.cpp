// Purpose: Implements the OpenMP parallel version for log analysis.

#include "../include/log_parser.h"
#include "../include/analyzer_utils.h"
#include "../include/timer.h"
#include <iostream>
using namespace std;

int main() {
    vector<string> files = {"../data/log1.txt", "../data/log2.txt"};
    vector<string> logs = readMultipleLogs(files);

    Timer t;
    auto result = analyzeLogsParallel(logs);  // Parallelized with OpenMP
    double time = t.elapsed();

    cout << "\n=== PARALLEL ANALYSIS ===\n";
    displayResults(result);
    cout << "\nExecution Time: " << time << " seconds\n";
    return 0;
}
