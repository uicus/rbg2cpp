#!/bin/bash 
# Flat-MC customsplit test for statistics.
# Usage:
# ./run_sim.sh [game] [simulations]

echo "Running simulation customsplit for $1 with simulations $2"
make simulate_customsplit_$1 SIMULATIONS=$2 > logs/$1-simcustomsplit.txt 2>&1
