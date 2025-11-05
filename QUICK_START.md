\# Quick Start

1) Build all binaries
```
make all
```

2) Generate RAW logs
```
python3 data/log_generator.py
```

3) Preprocess RAW -> CLEAN (parallel)
```
make preprocess-data
# or
make run-preprocessor
```

4) Run analyzers
```
make run-serial      # Serial analysis on default sample files
make run-parallel    # Parallel analysis on default sample files
make run-main        #  Interactive mode
```

5) Run benchmark comparison (serial vs parallel)
```
make run-bench       # Simple benchmark
bin/run_benchmark    # CSV-style output for plotting (if built)
```

6) Generate graphs (from results/benchmark_data.csv)
```
python3 scripts/plot_performance.py   # Serial vs Parallel times
python3 scripts/plot_speedup.py       # Speedup vs ideal
python3 scripts/plot_efficiency.py    # Efficiency (%)
```

All outputs:
- Clean logs: `data/clean/`
- CSV + graphs: `results/`
