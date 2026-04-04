#!/bin/bash

OUT_FILE="experiment_results.txt"
echo "K,IdxPages,DataPages,Threads,TotalOps,Ins%,Srch%,Del%,TimeMs,InsOk,InsFail,DelOk,DelFail,SrchHit,SrchMiss,AvgInsMs,AvgSrchMs,AvgDelMs" > $OUT_FILE

# Function to run a single experiment configuration
# Usage: run_expt <K> <INDEX_PAGES> <DATA_PAGES> <THREADS> <OPS_PER_THREAD> <INS_PCT> <SRCH_PCT>
run_expt() {
    K=$1
    IP=$2
    DP=$3
    TH=$4
    OPS=$5
    INS=$6
    SRCH=$7
    
    echo "Running with K=$K, IP=$IP, DP=$DP, Threads=$TH, Ops/Th=$OPS, Ins%=$INS, Srch%=$SRCH"
    
    # Clear the DB before each run
    make clean_db > /dev/null 2>&1 || true
    
    # Run bptree in experiment mode
    ./build/bptree --experiment $K $IP $DP $TH $OPS $INS $SRCH >> $OUT_FILE
}

# --- Define your specific experiments here ---
# Call format: run_expt K INDEX_PAGES DATA_PAGES THREADS OPS_PER_THREAD INS_PCT SRCH_PCT

# Iteration 1
run_expt 2 100 1000 4 25000 45 35

# Iteration 2
run_expt 1 50 500 8 12500 33 34

# Iteration 3
run_expt 5 500 5000 1 100000 80 10

# Add as many individual iterations as you need!

echo "Experiments finished. Results in $OUT_FILE"
