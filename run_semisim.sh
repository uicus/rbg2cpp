#!/bin/bash 
# Flat-MC test for statistics.
# Usage:
# ./run_sim.sh [game] [number_of_simulations]
make simulate_semisplit_$1 SIMULATIONS=$2 > logs/$1-simulationsemisplit.txt 2>&1
