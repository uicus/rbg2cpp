#!/bin/bash 
# Flat-MC test for statistics.
# Usage:
# ./run_sim.sh [game] [number_of_simulations]
echo "Running simulation for $1"
make simulate_$1 SIMULATIONS=$2 > logs/$1-simulation.txt 2>&1
