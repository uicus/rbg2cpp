#!/bin/bash 
make perft_$1 DEPTH=$2 > logs/perft-$1.txt 2>&1
