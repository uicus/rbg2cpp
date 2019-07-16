#!/bin/bash 
make simulate_$1 SIMULATIONS=$2 > logs/$1-simulation.txt 2>&1
