import matplotlib.pyplot as plt

sim1_k_values = [0, 1, 2, 3, 4, 5]

# Operation times (in microseconds)
sim1_avg_insert_time = [12.5, 11.2, 10.5, 10.3, 10.2, 10.1]
sim1_avg_delete_time = [14.0, 12.5, 11.8, 11.5, 11.4, 11.3]
sim1_avg_search_time = [8.0, 6.5, 5.2, 4.8, 4.6, 4.5]

# Hit ratios (percentage)
sim1_index_hit_ratio = [0.40, 0.55, 0.68, 0.72, 0.75, 0.76]
sim1_data_hit_ratio  = [0.30, 0.45, 0.55, 0.60, 0.62, 0.63]

# ---------------------------------------------------------------------
# Simulation 2: Varying Index Cache Frames (using best K)
# Array of index cache frames sizes
# ---------------------------------------------------------------------
sim2_frames = [50, 100, 200, 400, 800]

# Operation times (in microseconds)
sim2_avg_insert_time = [15.0, 10.5, 8.2, 6.5, 5.0]
sim2_avg_delete_time = [16.5, 11.8, 9.0, 7.2, 5.5]
sim2_avg_search_time = [9.0, 5.2, 3.5, 2.2, 1.5]

# Hit ratios (percentage)
sim2_index_hit_ratio = [0.35, 0.68, 0.82, 0.91, 0.98]

# ---------------------------------------------------------------------
# Simulation 3: Varying Number of Threads
# Fixed parameters: best K, 100 index cache frames
# ---------------------------------------------------------------------
sim3_threads = [1, 2, 4, 8]

# Average operation time across threads (in microseconds)
sim3_avg_op_time = [10.5, 5.8, 3.2, 1.8]


# =====================================================================
# PLOTTING SECTION
# This logic will automatically generate and save the 5 graphs
# =====================================================================

def plot_sim1_times():
    plt.figure(figsize=(8, 5))
    plt.plot(sim1_k_values, sim1_avg_insert_time, marker='o', label='Avg Insert Time')
    plt.plot(sim1_k_values, sim1_avg_delete_time, marker='s', label='Avg Delete Time')
    plt.plot(sim1_k_values, sim1_avg_search_time, marker='^', label='Avg Search Time')
    
    plt.title('Sim 1: LRU-K parameter (K) vs Avg Operation Time', fontsize=12)
    plt.xlabel('K (for LRU-K)')
    plt.ylabel('Average Time (in microseconds)')
    plt.xticks(sim1_k_values)
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig('sim1_times.png')
    plt.close()
    print("Saved 'sim1_times.png'")

def plot_sim1_hit_ratios():
    plt.figure(figsize=(8, 5))
    plt.plot(sim1_k_values, sim1_index_hit_ratio, marker='o', label='Index Cache Hit Ratio')
    plt.plot(sim1_k_values, sim1_data_hit_ratio, marker='s', label='Data Cache Hit Ratio')
    
    plt.title('Sim 1: LRU-K parameter (K) vs Cache Hit Ratios', fontsize=12)
    plt.xlabel('K (for LRU-K)')
    plt.ylabel('Hit Ratio (percentage)')
    plt.xticks(sim1_k_values)
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig('sim1_hit_ratios.png')
    plt.close()
    print("Saved 'sim1_hit_ratios.png'")

def plot_sim2_times():
    plt.figure(figsize=(8, 5))
    plt.plot(sim2_frames, sim2_avg_insert_time, marker='o', label='Avg Insert Time')
    plt.plot(sim2_frames, sim2_avg_delete_time, marker='s', label='Avg Delete Time')
    plt.plot(sim2_frames, sim2_avg_search_time, marker='^', label='Avg Search Time')
    
    plt.title('Sim 2: Index Cache Frames vs Avg Operation Time', fontsize=12)
    plt.xlabel('# Index Cache Frames')
    plt.ylabel('Average Time (in microseconds)')
    plt.xticks(sim2_frames)
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig('sim2_times.png')
    plt.close()
    print("Saved 'sim2_times.png'")

def plot_sim2_hit_ratios():
    plt.figure(figsize=(8, 5))
    plt.plot(sim2_frames, sim2_index_hit_ratio, marker='o', color='purple', label='Index Cache Hit Ratio')
    
    plt.title('Sim 2: Index Cache Frames vs Index Cache Hit Ratio', fontsize=12)
    plt.xlabel('# Index Cache Frames')
    plt.ylabel('Hit Ratio (percentage)')
    plt.xticks(sim2_frames)
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig('sim2_hit_ratio.png')
    plt.close()
    print("Saved 'sim2_hit_ratio.png'")

def plot_sim3_threads():
    plt.figure(figsize=(8, 5))
    plt.plot(sim3_threads, sim3_avg_op_time, marker='o', color='green', label='Avg Operation Time')
    
    plt.title('Sim 3: Number of Threads vs Avg Operation Time', fontsize=12)
    plt.xlabel('# Threads')
    plt.ylabel('Average Time (in microseconds)')
    plt.xticks(sim3_threads)
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig('sim3_threads.png')
    plt.close()
    print("Saved 'sim3_threads.png'")

if __name__ == '__main__':
    print("Generating graphs...")
    plot_sim1_times()
    plot_sim1_hit_ratios()
    plot_sim2_times()
    plot_sim2_hit_ratios()
    plot_sim3_threads()
    print("All 5 graphs generated successfully in the current directory.")
