#!/bin/bash
# Example flat-MC benchmark with 1s per game.

mkdir logs

./run_benchmark.sh amazons 1000
./run_benchmark.sh breakthrough 1000
./run_benchmark.sh chess 1000
./run_benchmark.sh englishDraughts 1000
./run_benchmark.sh hex 1000
./run_benchmark.sh internationalDraughts 1000
./run_benchmark.sh skirmish 1000
