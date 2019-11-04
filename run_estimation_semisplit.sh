#!/bin/bash 
# Flat-MC test for statistics.
# Usage:
# ./run_estimation_semisim.sh [game] [time in ms for single estimation]
make estimate_semisplit_$1 SIMULATIONS=$2 > logs/$1-estimation.txt 2>&1
