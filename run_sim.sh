#!/bin/bash 
# Flat-MC test with statistics.
# Usage:
# ./run_sim.sh [game] [simulations] [random_generator]
# [game] -- the name of the game, e.g, breakthrough
# [simulations] -- the number of random simulations
# [random_generator] -- random generator id:
#   1 -- reimplementation of the Java standard generator
#   2 -- Lemire's enhanced method
#   other -- default method with std::mt19937

echo "Running simulations for $1 with simulations $2 randgen $3"
make simulate_$1 SIMULATIONS=$2 RANDGEN=$3 > logs/$1-sim.txt 2>&1
