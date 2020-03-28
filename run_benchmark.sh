#!/bin/bash 
# Flat-MC tester for speed.
# Usage:
# ./run_benchmark.sh [game] [time] [random_generator]
# [game] -- the name of the game, e.g, breakthrough
# [time] -- time in ms, without preprocessing
# [random_generator] -- random generator id:
#   0 -- default method with std::mt19937
#   1 -- reimplementation of the Java standard generator
#   2 -- Lemire's enhanced method

if [[ $3 == 1 ]] || [[ $3 == 2 ]]; then
randgen=$3
else
randgen=0
fi
echo "Running benchmark for $1 with time $2 randgen $randgen"
make benchmark_$1 TIME=$2 RANDGEN=$randgen > logs/$1-benchmark.txt 2>&1
