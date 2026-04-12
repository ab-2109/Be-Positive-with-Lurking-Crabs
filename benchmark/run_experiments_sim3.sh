#!/usr/bin/env bash

set -euo pipefail

OUT_FILE="experiment_results_sim3.txt"
echo "K,IdxPages,DataPages,Threads,TotalOps,Ins%,Srch%,Del%,TimeMs,InsOk,InsFail,DelOk,DelFail,SrchHit,SrchMiss,AvgInsMs,AvgSrchMs,AvgDelMs" > "$OUT_FILE"

DATASET_FILE="apartments_for_rent_classified_100K.csv"
K_VAL=2
INDEX_PAGES=100
DATA_PAGES=1000
TOTAL_OPS=100000
INS_PCT=30
SRCH_PCT=40
DEL_PCT=30
CPUSET="${CPUSET:-0-7}"
THREADS_LIST=(${THREADS_LIST:-1 2 4 8})

if ! command -v taskset >/dev/null 2>&1; then
    echo "taskset is required for CPU affinity pinning but was not found." >&2
    exit 1
fi

if ! taskset -cp "$CPUSET" $$ >/dev/null 2>&1; then
    echo "Warning: CPU set '$CPUSET' may not be valid on this machine. Check \`nproc --all\` and \`lscpu -e\`." >&2
fi


run_expt() {
    K=$1
    IP=$2
    DP=$3
    TH=$4
    OPS=$5
    INS=$6
    SRCH=$7
    DEL=$8
    
    echo "Running with K=$K, IP=$IP, DP=$DP, Threads=$TH, Ops/Th=$OPS, Ins%=$INS, Srch%=$SRCH, Del%=$DEL"
    
    make clean_db > /dev/null 2>&1 || true

    printf "6\n7\n" | taskset -c "$CPUSET" ./build/bptree > /dev/null 2>&1
    
    taskset -c "$CPUSET" ./build/bptree --experiment "$K" "$IP" "$DP" "$TH" "$OPS" "$INS" "$SRCH" "$DEL" >> "$OUT_FILE"
}

for THREADS in "${THREADS_LIST[@]}"; do
    OPS_PER_THREAD=$((TOTAL_OPS / THREADS))
    run_expt "$K_VAL" "$INDEX_PAGES" "$DATA_PAGES" "$THREADS" "$OPS_PER_THREAD" "$INS_PCT" "$SRCH_PCT" "$DEL_PCT"
done

echo "Experiments finished. Results in $OUT_FILE"
