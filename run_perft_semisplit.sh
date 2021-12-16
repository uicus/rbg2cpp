#!/bin/bash
# Perft semisplit for a specified depth.
# Usage:
# ./run_perft_semisplit.sh [game] [depth]

echo "Running perft semisplit on actions for $1 with depth $2"
make perft_semisplit_$1 DEPTH=$2 > logs/$1-perft_semisplita.txt 2>&1
