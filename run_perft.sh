#!/bin/bash
# Perft for a specified depth.
# Usage:
# ./run_perft.sh [game] [depth]
make perft_$1 DEPTH=$2 > logs/$1-perft.txt 2>&1
