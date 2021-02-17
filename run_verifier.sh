#!/bin/bash 
# Usage:
# ./run_verifier.sh [game] [simulations]

make verify_$1 SIMULATIONS=$2 RANDGEN=0 > logs/$1-verify.txt 2>&1
