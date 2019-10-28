#!/bin/bash 
# Flat-MC semisplit tester for speed.
# Usage:
# ./run_benchmark.sh [game] [time]
make benchmark_semisplit_$1 SIMULATIONS=$2 > logs/$1-benchmark_semisplit.txt 2>&1
