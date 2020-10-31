#!/bin/bash 
# Flat-MC customsplit test for speed.
# Usage:
# ./run_benchmark.sh [game] [time]

echo "Running benchmark customsplit for $1 with time $2"
make benchmark_customsplit_$1 TIME=$2 > logs/$1-benchmarkcustomsplit$3.txt 2>&1
