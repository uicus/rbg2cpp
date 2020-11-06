#!/bin/bash 
# Flat-MC semisplit test for speed.
# Usage:
# ./run_benchmark.sh [game] [time]

echo "Running benchmark semisplit for $1 with time $2"
make benchmark_semisplit_$1 TIME=$2 > logs/$1-benchmarksemisplit.txt 2>&1
