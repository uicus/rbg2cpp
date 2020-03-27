#!/bin/bash 
# Flat-MC tester for speed.
# Usage:
# ./run_benchmark.sh [game] [time] [random_generator]
# [game] -- the name of the game, e.g, breakthrough
# [time] -- time in ms, without preprocessing
# [random_generator] -- random generator id:
#   1 -- reimplementation of the Java standard generator
#   2 -- Lemire's enhanced method
#   other -- default method with std::mt19937

echo "Running benchmark for $1 with time $2 randgen $3"
make benchmark_$1 TIME=$2 RANDGEN=$3 > logs/$1-benchmark.txt 2>&1
