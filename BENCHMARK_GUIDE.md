# Benchmark and Visualization Guide

This guide explains how to generate benchmark data and create performance visualization graphs for the Parallel Log Analyzer project.

## Quick Start

### 1. Install Python Dependencies

```bash
pip install pandas matplotlib numpy
```

Or using the requirements file:
```bash
pip install -r scripts/requirements.txt
```

### 2. Generate Log Files

The log generator creates log files of different sizes for comprehensive benchmarking:

```bash
python3 data/log_generator.py
```

This creates:
- `log_small.txt` (100 lines)
- `log_medium.txt` (1,000 lines)
- `log_large.txt` (10,000 lines)
- `log_xlarge.txt` (50,000 lines)
- `log_xxlarge.txt` (100,000 lines)

### 3. Build the Benchmark Executable

```bash
make all
```

Or manually:
```bash
g++ -std=c++17 -fopenmp -O2 -I./include src/run_benchmark.cpp -o bin/run_benchmark
```

### 4. Run Complete Benchmark Suite

**Option A: Using the master Python script (recommended)**
```bash
python3 scripts/run_all_benchmarks.py
```

This will:
1. Generate log files
2. Build benchmark executable
3. Run benchmarks
4. Generate all visualization graphs

**Option B: Manual step-by-step**

1. Run the benchmark:
```bash
make run-bench-all
```

2. Generate graphs individually:
```bash
python3 scripts/plot_performance.py
python3 scripts/plot_speedup.py
python3 scripts/plot_efficiency.py
```

## Output Files

All results are saved in the `results/` directory:

- **benchmark_data.csv** - Raw performance data (CSV format)
- **serial_vs_parallel.png** - Performance comparison graph
- **speedup_analysis.png** - Speedup analysis graph
- **efficiency_analysis.png** - Efficiency analysis graph

## Understanding the Graphs

### 1. Performance Comparison (serial_vs_parallel.png)
- Shows execution time vs number of log lines
- Compare how serial and parallel implementations perform
- Log-log scale provides better visualization for large datasets

### 2. Speedup Analysis (speedup_analysis.png)
- Shows how much faster parallel implementation is
- Formula: `Speedup = Serial Time / Parallel Time`
- Includes ideal speedup line (4x for 4 threads)
- Measures parallelization effectiveness

### 3. Efficiency Analysis (efficiency_analysis.png)
- Shows how efficiently CPU cores are utilized
- Formula: `Efficiency = (Speedup / Number of Threads) × 100%`
- Ideal efficiency: 100% (perfect parallelization)
- Lower efficiency indicates overhead (synchronization, load imbalance, etc.)

## Interpreting Results

### Good Results:
- **Speedup ≈ 4x** with 4 threads = Excellent parallelization
- **Efficiency > 75%** = Good utilization of CPU cores
- **Scaling well** = Performance improves proportionally with data size

### Issues to Look For:
- **Low speedup (< 2x)** = Insufficient parallelism or high overhead
- **Low efficiency (< 50%)** = Poor thread utilization
- **Performance degrades with size** = Scalability issues

## Troubleshooting

### Issue: Python script fails to import libraries
**Solution:** Install required packages
```bash
pip install pandas matplotlib numpy
```

### Issue: Benchmark executable not found
**Solution:** Build the benchmark
```bash
make all
# or manually:
g++ -std=c++17 -fopenmp -O2 -I./include src/run_benchmark.cpp -o bin/run_benchmark
```

### Issue: OpenMP errors
**Solution:** Ensure your compiler supports OpenMP and the -fopenmp flag is set
```bash
g++ --version  # Should show version info
```

### Issue: No graphs appear
**Solution:** Check if data files exist
```bash
ls results/benchmark_data.csv
```

If missing, run the benchmark first:
```bash
make run-bench-all
```

## Advanced Usage

### Custom Benchmark Configuration

Edit `src/run_benchmark.cpp` to:
- Change number of runs for averaging
- Adjust number of threads
- Modify log files to benchmark

### Custom Graph Settings

Edit individual plotting scripts to:
- Change figure size and DPI
- Modify colors and styles
- Adjust annotations and labels

## File Structure

```
project/
├── data/
│   ├── log_generator.py          # Generates test log files
│   ├── log1.txt, log2.txt       # Original small test files
│   └── log_*.txt                 # Generated benchmark files
├── src/
│   └── run_benchmark.cpp         # Benchmark executable source
├── scripts/
│   ├── run_all_benchmarks.py    # Master automation script
│   ├── plot_performance.py       # Performance comparison
│   ├── plot_speedup.py          # Speedup analysis
│   ├── plot_efficiency.py       # Efficiency analysis
│   ├── README.md                # Scripts documentation
│   └── requirements.txt         # Python dependencies
└── results/
    ├── benchmark_data.csv       # Generated benchmark data
    └── *.png                    # Generated graphs
```

## Next Steps

After generating graphs, you can:
1. Include them in documentation
2. Add to presentations or reports
3. Analyze performance bottlenecks
4. Optimize the parallel implementation
5. Experiment with different thread counts

