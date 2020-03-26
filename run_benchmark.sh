#!/bin/bash 
# Flat-MC tester for speed.
# Usage:
# ./run_benchmark.sh [game] [time]
echo "Running benchmark for $1"
make benchmark_$1 TIME=$2 > logs/$1-benchmark.txt 2>&1
