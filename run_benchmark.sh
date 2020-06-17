#!/bin/bash 
# Flat-MC tester for speed.
# Usage:
# ./run_benchmark.sh [game] [time] [random_generator]
# [game] -- the name of the game, e.g, breakthrough
# [time] -- time in ms, without preprocessing
# [random_generator] -- random generator id (see test/rbg_random_generator.hpp)

if [[ -z "$3" ]]; then
randgen=0
else
randgen=$3
fi
echo "Running benchmark for $1 with time $2 randgen $randgen"
make benchmark_$1 TIME=$2 RANDGEN=$randgen > logs/$1-benchmark.txt 2>&1
