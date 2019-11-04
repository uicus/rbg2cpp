#!/bin/bash 
# Flat-MC tester for speed.
# Usage:
# ./run_benchmark.sh [game] [time]
make benchmark_$1 SIMULATIONS=$2 > logs/$1-benchmark.txt 2>&1
