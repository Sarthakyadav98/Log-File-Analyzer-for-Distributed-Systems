#!/usr/bin/env python3
"""
Plot trend analysis from performance_report.csv
Shows time-based log activity, stress scores, and anomalies
"""

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime
import numpy as np
import os

def load_data(csv_file="results/output/performance_report.csv"):
    """Load the performance report CSV data"""
    try:
        df = pd.read_csv(csv_file)
        # Convert TimeSlot to datetime for better plotting
        df['DateTime'] = pd.to_datetime(df['TimeSlot'] + ':00:00')
        return df
    except FileNotFoundError:
        print(f"Error: {csv_file} not found. Please run the analyzer first.")
        return None
    except Exception as e:
        print(f"Error loading data: {e}")
        return None

def plot_log_activity_trend(df):
    """Plot log activity over time"""
    plt.figure(figsize=(15, 8))
    
    # Create subplots
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(15, 12), sharex=True)
    
    # Plot 1: Log Count Over Time
    ax1.plot(df['DateTime'], df['LogCount'], 'b-o', linewidth=2, markersize=4, label='Log Count')
    
    # Highlight anomalies
    anomalies = df[df['IsAnomaly'] == 1]
    if not anomalies.empty:
        ax1.scatter(anomalies['DateTime'], anomalies['LogCount'], 
                   color='red', s=100, alpha=0.7, label='Anomalies', zorder=5)
    
    ax1.set_ylabel('Log Count', fontsize=12, fontweight='bold')
    ax1.set_title('System Log Activity Trend Analysis', fontsize=14, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    
    # Plot 2: Stress Score Over Time
    ax2.plot(df['DateTime'], df['StressScore'], 'r-o', linewidth=2, markersize=4, label='Stress Score')
    ax2.fill_between(df['DateTime'], df['StressScore'], alpha=0.3, color='red')
    
    # Add average line
    avg_stress = df['StressScore'].mean()
    ax2.axhline(y=avg_stress, color='orange', linestyle='--', linewidth=2, label=f'Average ({avg_stress:.1f})')
    
    ax2.set_ylabel('Stress Score', fontsize=12, fontweight='bold')
    ax2.set_title('System Stress Score Over Time', fontsize=14, fontweight='bold')
    ax2.grid(True, alpha=0.3)
    ax2.legend()
    
    # Plot 3: Combined View (Normalized)
    # Normalize both metrics to 0-1 scale for comparison
    log_norm = (df['LogCount'] - df['LogCount'].min()) / (df['LogCount'].max() - df['LogCount'].min())
    stress_norm = (df['StressScore'] - df['StressScore'].min()) / (df['StressScore'].max() - df['StressScore'].min())
    
    ax3.plot(df['DateTime'], log_norm, 'b-', linewidth=2, label='Log Activity (normalized)', alpha=0.7)
    ax3.plot(df['DateTime'], stress_norm, 'r-', linewidth=2, label='Stress Score (normalized)', alpha=0.7)
    ax3.fill_between(df['DateTime'], log_norm, alpha=0.2, color='blue')
    ax3.fill_between(df['DateTime'], stress_norm, alpha=0.2, color='red')
    
    ax3.set_ylabel('Normalized Values', fontsize=12, fontweight='bold')
    ax3.set_xlabel('Time', fontsize=12, fontweight='bold')
    ax3.set_title('Normalized Comparison: Activity vs Stress', fontsize=14, fontweight='bold')
    ax3.grid(True, alpha=0.3)
    ax3.legend()
    
    # Format x-axis
    for ax in [ax1, ax2, ax3]:
        ax.xaxis.set_major_formatter(mdates.DateFormatter('%m-%d %H:%M'))
        ax.xaxis.set_major_locator(mdates.HourLocator(interval=max(1, len(df)//10)))
        plt.setp(ax.xaxis.get_majorticklabels(), rotation=45)
    
    plt.tight_layout()
    
    # Save the plot
    output_dir = 'results/output'
    os.makedirs(output_dir, exist_ok=True)
    plt.savefig('results/output/trend_analysis.png', dpi=300, bbox_inches='tight')
    print("Trend analysis saved to: results/output/trend_analysis.png")
    
    plt.show()

def plot_hourly_heatmap(df):
    """Create a heatmap showing activity patterns by hour of day"""
    plt.figure(figsize=(12, 8))
    
    # Extract hour and date
    df['Hour'] = df['DateTime'].dt.hour
    df['Date'] = df['DateTime'].dt.date
    
    # Create pivot table for heatmap
    pivot_data = df.pivot_table(values='LogCount', index='Date', columns='Hour', fill_value=0)
    
    # Create heatmap
    plt.imshow(pivot_data.values, cmap='YlOrRd', aspect='auto', interpolation='nearest')
    plt.colorbar(label='Log Count')
    
    # Set labels
    plt.xlabel('Hour of Day', fontsize=12, fontweight='bold')
    plt.ylabel('Date', fontsize=12, fontweight='bold')
    plt.title('Log Activity Heatmap by Hour', fontsize=14, fontweight='bold')
    
    # Set ticks
    plt.xticks(range(24), [f'{h:02d}:00' for h in range(24)], rotation=45)
    
    # Format y-axis to show dates
    date_labels = [str(date) for date in pivot_data.index[::max(1, len(pivot_data)//10)]]
    date_positions = list(range(0, len(pivot_data), max(1, len(pivot_data)//10)))
    plt.yticks(date_positions, date_labels)
    
    plt.tight_layout()
    
    # Save the plot
    plt.savefig('results/output/activity_heatmap.png', dpi=300, bbox_inches='tight')
    print("Activity heatmap saved to: results/output/activity_heatmap.png")
    
    plt.show()

def generate_summary_stats(df):
    """Generate and print summary statistics"""
    print("\n" + "="*60)
    print("TREND ANALYSIS SUMMARY")
    print("="*60)
    
    print(f"Total Time Periods Analyzed: {len(df)}")
    print(f"Total Log Entries: {df['LogCount'].sum():,}")
    print(f"Average Logs per Hour: {df['LogCount'].mean():.1f}")
    print(f"Peak Activity: {df['LogCount'].max():,} logs")
    print(f"Peak Activity Time: {df.loc[df['LogCount'].idxmax(), 'TimeSlot']}:00")
    
    print(f"\nStress Score Analysis:")
    print(f"Average Stress Score: {df['StressScore'].mean():.1f}")
    print(f"Peak Stress Score: {df['StressScore'].max()}")
    print(f"Peak Stress Time: {df.loc[df['StressScore'].idxmax(), 'TimeSlot']}:00")
    
    anomaly_count = df['IsAnomaly'].sum()
    print(f"\nAnomaly Detection:")
    print(f"Anomalous Periods Detected: {anomaly_count}")
    if anomaly_count > 0:
        anomaly_times = df[df['IsAnomaly'] == 1]['TimeSlot'].tolist()
        print(f"Anomaly Times: {', '.join([t + ':00' for t in anomaly_times[:5]])}")
        if len(anomaly_times) > 5:
            print(f"... and {len(anomaly_times) - 5} more")

def main():
    print("Loading trend analysis data...")
    
    # Load data
    df = load_data()
    if df is None:
        return
    
    print(f"Loaded {len(df)} time periods for analysis")
    
    # Generate summary statistics
    generate_summary_stats(df)
    
    # Create visualizations
    print("\nGenerating trend analysis plots...")
    plot_log_activity_trend(df)
    
    print("\nGenerating activity heatmap...")
    plot_hourly_heatmap(df)
    
    print("\nTrend analysis complete!")
    print("Generated files:")
    print("  - results/output/trend_analysis.png")
    print("  - results/output/activity_heatmap.png")

if __name__ == "__main__":
    main()