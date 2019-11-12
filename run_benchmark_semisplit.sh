#!/bin/bash 
# Flat-MC semisplit tester for speed.
# Usage:
# ./run_benchmark.sh [game] [time] [semilength]
make benchmark_semisplit_$1 TIME=$2 SEMILENGTH=$3 > logs/$1-benchmarksemisplit$3.txt 2>&1
