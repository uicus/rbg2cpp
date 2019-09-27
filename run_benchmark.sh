#!/bin/bash 
# Flat-MC tester for speed.
# Usage:
# ./run_benchmark.sh [game] [number_of_simulations]
make benchmark_$1 SIMULATIONS=$2 > logs/$1-benchmark.txt 2>&1
