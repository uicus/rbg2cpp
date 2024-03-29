#!/bin/bash 
# Flat-MC semisplit test for statistics.
# Usage:
# ./run_sim.sh [game] [simulations]

echo "Running simulation semisplit on actions for $1 with simulations $2"
make simulate_semisplit_$1 SIMULATIONS=$2 > logs/$1-simsemisplita.txt 2>&1
