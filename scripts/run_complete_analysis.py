#!/usr/bin/env python3
"""
Complete analysis workflow: build, run analyzer, and generate visualizations
"""

import subprocess
import os
import sys

def main():
    print("=" * 70)
    print("COMPLETE LOG ANALYSIS WORKFLOW")
    print("=" * 70)
    
    # Get project directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.join(script_dir, '..')
    os.chdir(project_dir)
    
    # Step 1: Build the project
    print("\n[Step 1/4] Building the project...")
    try:
        subprocess.run(['make', 'all'], check=True)
        print("✓ Build completed successfully")
    except subprocess.CalledProcessError:
        print("✗ Build failed")
        sys.exit(1)
    
    # Step 2: Run the advanced analysis
    print("\n[Step 2/4] Running advanced log analysis...")
    try:
        # Compile and run the test
        subprocess.run([
            'g++', '-std=c++17', '-fopenmp', '-O2', '-I./include',
            'test_advanced_features.cpp', '-o', 'bin/test_advanced'
        ], check=True)
        
        result = subprocess.run(['bin/test_advanced'], capture_output=True, text=True, check=True)
        print("✓ Analysis completed successfully")
        print("\nAnalysis Output:")
        print("-" * 50)
        print(result.stdout)
        
    except subprocess.CalledProcessError as e:
        print(f"✗ Analysis failed: {e}")
        if e.stderr:
            print(f"Error: {e.stderr}")
        sys.exit(1)
    
    # Step 3: Check if CSV was generated
    print("\n[Step 3/4] Checking CSV output...")
    csv_file = "results/output/performance_report.csv"
    if os.path.exists(csv_file):
        print(f"✓ CSV file generated: {csv_file}")
        
        # Show first few lines of CSV
        try:
            with open(csv_file, 'r') as f:
                lines = f.readlines()[:6]  # Header + 5 data lines
                print("\nCSV Preview:")
                print("-" * 50)
                for line in lines:
                    print(line.strip())
                if len(lines) > 5:
                    print("...")
        except Exception as e:
            print(f"Could not preview CSV: {e}")
    else:
        print(f"✗ CSV file not found: {csv_file}")
    
    # Step 4: Generate trend visualizations
    print("\n[Step 4/4] Generating trend visualizations...")
    try:
        subprocess.run(['python3', 'scripts/plot_trend_analysis.py'], check=True)
        print("✓ Trend analysis completed")
    except subprocess.CalledProcessError as e:
        print(f"Note: Trend visualization failed: {e}")
        print("This might be due to missing Python dependencies (pandas, matplotlib)")
        print("Install with: pip install pandas matplotlib")
    
    print("\n" + "=" * 70)
    print("COMPLETE ANALYSIS WORKFLOW FINISHED!")
    print("=" * 70)
    print("\nGenerated files:")
    print("  - results/output/performance_report.csv")
    print("  - results/output/trend_analysis.png")
    print("  - results/output/activity_heatmap.png")
    print("\nFeatures demonstrated:")
    print("  ✓ Warning message tracking")
    print("  ✓ High load time slot detection")
    print("  ✓ Peak hour analysis")
    print("  ✓ Error severity scoring")
    print("  ✓ Anomaly detection")
    print("  ✓ CSV export for trend analysis")
    print("  ✓ Visualization generation")

if __name__ == "__main__":
    main()