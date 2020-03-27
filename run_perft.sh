#!/bin/bash
# Perft for a specified depth.
# Usage:
# ./run_perft.sh [game] [depth]

echo "Running perft for $1 with depth $2"
make perft_$1 DEPTH=$2 > logs/$1-perft.txt 2>&1
