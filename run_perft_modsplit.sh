#!/bin/bash
# Perft for a specified depth.
# Usage:
# ./run_perft_modsplit.sh [game] [depth] [semilength]

echo "Running perft modsplit for $1 with depth $2 semilength $3"
make perft_modsplit_$1 DEPTH=$2 SEMILENGTH=$3 > logs/$1-perft_semisplit$3.txt 2>&1
