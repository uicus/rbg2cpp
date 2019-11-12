#!/bin/bash
# Perft for a specified depth.
# Usage:
# ./run_perft_semisplit.sh [game] [depth] [semilength]
make perft_semisplit_$1 DEPTH=$2 SEMILENGTH=99999 > logs/$1-perft_semisplit$3.txt 2>&1
