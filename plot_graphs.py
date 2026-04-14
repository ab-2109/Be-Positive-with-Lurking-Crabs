import matplotlib.pyplot as plt

sim1_k_values = [0, 1, 2, 3, 4, 5]

# Operation times (in milliseconds)
sim1_avg_insert_time = [3.6857, 3.51527, 4.06126, 4.41454, 4.77187, 5.5107]
sim1_avg_search_time = [3.87209, 3.68265, 3.40539, 3.40116, 3.56143, 4.47333]
sim1_avg_delete_time = [3.57584, 3.41711, 3.44431, 3.41917, 3.60601, 4.4556]

# Hit ratios (percentage)
sim1_index_hit_ratio = [58.52, 58.80, 53.89, 50.78, 47.63, 51.96]
sim1_data_hit_ratio  = [1.15, 0.99, 0.92, 1.1, 1.05, 1.12]

# ---------------------------------------------------------------------
# Simulation 2: Varying Index Cache Frames (using best K)
# Array of index cache frames sizes
# ---------------------------------------------------------------------
sim2_frames = [0, 50, 100, 200, 400, 800]

# Operation times (in milliseconds)
sim2_avg_insert_time = [7.47625, 4.3547, 4.05614, 3.69698, 3.38562,2.86119]
sim2_avg_search_time = [4.89192, 3.49725, 3.33162, 3.1205, 2.98083, 2.66956]
sim2_avg_delete_time = [5.9663, 3.5541, 3.40961, 3.20983, 3.08839, 2.7828]

# Hit ratios (percentage)
sim2_index_hit_ratio = [0, 48.46, 53.74, 58.18, 63.89, 70.03]

# ---------------------------------------------------------------------
# Simulation 3: Varying Number of Threads
# Fixed parameters: best K, 100 index cache frames
# ---------------------------------------------------------------------
sim3_threads = [1, 2, 4, 8]

# Average operation time across threads (in milliseconds)
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
    plt.ylabel('Average Time (in milliseconds)')
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
    plt.ylabel('Average Time (in milliseconds)')
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
    plt.ylabel('Average Time (in seconds)')
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
