#!/bin/bash 
make simulate_$1 SIMULATIONS=$2 > logs/mc-$1.txt 2>&1
