// Purpose: Compares serial vs parallel performance on the same dataset.


#include "../include/log_parser.h"
#include "../include/analyzer_utils.h"
#include "../include/timer.h"
#include <iostream>
using namespace std;

int main() {
    vector<string> files = {"../data/log1.txt", "../data/log2.txt"};
    vector<string> logs = readMultipleLogs(files);

    Timer serialT;
    auto serialRes = analyzeLogsSerial(logs);
    double serialTime = serialT.elapsed();

    Timer parallelT;
    auto parallelRes = analyzeLogsParallel(logs);
    double parallelTime = parallelT.elapsed();

    cout << "\n=== BENCHMARK RESULTS ===\n";
    cout << "Serial Time   : " << serialTime << " s\n";
    cout << "Parallel Time : " << parallelTime << " s\n";
    cout << "Speedup       : " << serialTime / parallelTime << "x\n";
    cout << "Efficiency    : " << (serialTime / parallelTime) * 100 << " % (approx)\n";
    return 0;
}
