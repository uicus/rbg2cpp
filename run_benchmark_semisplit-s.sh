#!/bin/bash 
# Flat-MC semisplit test for speed.
# Usage:
# ./run_benchmark.sh [game] [time]

echo "Running benchmark semisplit on semimoves for $1 with time $2"
make benchmark_semisplit-s_$1 TIME=$2 > logs/$1-benchmarksemisplit-s.txt 2>&1
