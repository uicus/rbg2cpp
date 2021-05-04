#!/bin/bash 
# Flat-MC test with statistics.
# Usage:
# ./run_sim.sh [game] [simulations] [random_generator]
# [game] -- the name of the game, e.g, breakthrough
# [simulations] -- the number of random simulations
# [random_generator] -- random generator id (see test/rbg_random_generator.hpp)

if [[ -z "$3" ]]; then
randgen=0
else
randgen=$3
fi
echo "Running simulations for $1 with simulations $2 randgen $randgen"
make simulate_$1 SIMULATIONS=$2 RANDGEN=$randgen > logs/$1-sim.txt 2>&1
