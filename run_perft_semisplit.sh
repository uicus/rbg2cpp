#!/bin/bash
# Perft for a specified depth.
# Usage:
# ./run_perft_semisplit.sh [game] [depth] [semilength]
echo "Running perft semisplit for $1 with semilength $3"
make perft_semisplit_$1 DEPTH=$2 SEMILENGTH=$3 > logs/$1-perft_semisplit$3.txt 2>&1
