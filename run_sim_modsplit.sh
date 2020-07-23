#!/bin/bash 
# Flat-MC modsplit test for statistics.
# Usage:
# ./run_sim.sh [game] [simulations] [semilength]

echo "Running simulation modsplit for $1 with simulations $2 semilength $3"
make simulate_modsplit_$1 SIMULATIONS=$2 SEMILENGTH=$3 > logs/$1-simmodsplit$3.txt 2>&1
