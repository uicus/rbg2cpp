#!/bin/bash 
# Flat-MC test for statistics.
# Usage:
# ./run_sim.sh [game] [number_of_simulations] [semilength]
echo "Running simulation semisplit for $1 with semilength $3"
make simulate_semisplit_$1 SIMULATIONS=$2 SEMILENGTH=$3 > logs/$1-simsemisplit$3.txt 2>&1
