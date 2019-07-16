#!/bin/bash 
make perft_$1 DEPTH=$2 > logs/$1-perft.txt 2>&1
