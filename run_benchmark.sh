#!/bin/bash 
make benchmark_$1 SIMULATIONS=$2 > logs/$1-benchmark.txt 2>&1
