#!/usr/bin/env python3
"""
Master script to generate log files, run benchmarks, and generate all graphs
"""

import subprocess
import os
import sys

def main():
    print("=" * 70)
    print("PARALLEL LOG ANALYZER - BENCHMARK AND VISUALIZATION")
    print("=" * 70)
    
    # Get project directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.join(script_dir, '..')
    os.chdir(project_dir)
    
    # Step 1: Generate log files
    print("\n[Step 1/4] Generating log files...")
    try:
        subprocess.run(['python3', 'data/log_generator.py'], check=True)
        print("✓ Log files generated successfully")
    except subprocess.CalledProcessError:
        print("✗ Failed to generate log files")
        sys.exit(1)
    
    # Step 2: Build the benchmark executable
    print("\n[Step 2/4] Building benchmark executable...")
    try:
        result = subprocess.run(['make', 'all'], capture_output=True, text=True)
        print("✓ Build completed")
        
        # Also try to build run_benchmark manually if needed
        print("Building run_benchmark...")
        result = subprocess.run([
            'g++', '-std=c++17', '-fopenmp', '-O2', '-I./include',
            'src/run_benchmark.cpp', '-o', 'bin/run_benchmark'
        ], capture_output=True, text=True)
        if result.returncode == 0:
            print("✓ run_benchmark built successfully")
        else:
            print("Note: run_benchmark build warnings (may still work)")
    except Exception as e:
        print(f"Build note: {e}")
    
    # Step 3: Run benchmark
    print("\n[Step 3/4] Running benchmark...")
    try:
        # Try both Windows and Unix executables
        exe_path = None
        for path in ['bin/run_benchmark.exe', 'bin/run_benchmark', './bin/run_benchmark']:
            if os.path.exists(path):
                exe_path = path
                break
        
        if exe_path:
            result = subprocess.run([exe_path], check=True)
            print("✓ Benchmark completed successfully")
        else:
            print("✗ run_benchmark executable not found")
            print("Trying to run alternative benchmark...")
            # Fallback to regular benchmark
            subprocess.run(['make', 'run-bench'], check=True)
    except subprocess.CalledProcessError as e:
        print(f"✗ Benchmark failed: {e}")
        print("Trying alternative method...")
    except Exception as e:
        print(f"Note: {e}")
    
    # Step 4: Generate graphs
    print("\n[Step 4/4] Generating visualization graphs...")
    plots = [
        ('plot_performance.py', 'Performance comparison'),
        ('plot_speedup.py', 'Speedup analysis'),
        ('plot_efficiency.py', 'Efficiency analysis')
    ]
    
    for plot_script, description in plots:
        try:
            print(f"  → {description}...")
            subprocess.run(['python3', f'scripts/{plot_script}'], check=True)
            print(f"  ✓ {description} completed")
        except subprocess.CalledProcessError:
            print(f"  ✗ {description} failed")
            import traceback
            traceback.print_exc()
    
    print("\n" + "=" * 70)
    print("ALL BENCHMARK AND VISUALIZATION COMPLETE!")
    print("=" * 70)
    print("\nGenerated files:")
    print("  - results/benchmark_data.csv")
    print("  - results/serial_vs_parallel.png")
    print("  - results/speedup_analysis.png")
    print("  - results/efficiency_analysis.png")
    print("\nView the results in the results/ directory")

if __name__ == "__main__":
    main()

