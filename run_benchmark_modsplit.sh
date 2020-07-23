#!/bin/bash 
# Flat-MC modsplit test for speed.
# Usage:
# ./run_benchmark.sh [game] [time] [semilength]

echo "Running benchmark modsplit for $1 with time $2 semilength $3"
make benchmark_modsplit_$1 TIME=$2 SEMILENGTH=$3 > logs/$1-benchmarkmodsplit$3.txt 2>&1
