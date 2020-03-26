#!/bin/bash 
# Flat-MC semisplit tester for speed.
# Usage:
# ./run_benchmark.sh [game] [time] [semilength]
echo "Running benchmark semisplit for $1 with semilength $3"
make benchmark_semisplit_$1 TIME=$2 SEMILENGTH=$3 > logs/$1-benchmarksemisplit$3.txt 2>&1
