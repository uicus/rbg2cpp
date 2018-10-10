#!/bin/bash
mkdir logs

./run_mc.sh amazons 400000
./run_mc.sh amazons-split 3000000
./run_mc.sh arimaa 20
./run_mc.sh breakthrough 2600000
./run_mc.sh chess 120000
./run_mc.sh chess-kingCapture 1000000
./run_mc.sh chineseCheckers6 320000
./run_mc.sh connect4 60000000
./run_mc.sh doubleChess 8000
./run_mc.sh englishCheckers 4200000
./run_mc.sh englishCheckers-split 4000000
./run_mc.sh go 44000
./run_mc.sh gomoku 2000000
./run_mc.sh hex 4000000
./run_mc.sh hex_9x9 8000000
./run_mc.sh internationalCheckers 360000
./run_mc.sh paperSoccer 52000
./run_mc.sh reversi 1800000
./run_mc.sh theMillGame 1800000
./run_mc.sh ticTacToe 300000000

./run_perft.sh amazons 3
./run_perft.sh amazons-split 6
./run_perft.sh arimaa-fixedPosition 2
./run_perft.sh breakthrough 7
./run_perft.sh chess 6
./run_perft.sh chess-kingCapture 7
./run_perft.sh chineseCheckers6 6
./run_perft.sh connect4 10
./run_perft.sh doubleChess 4
./run_perft.sh englishCheckers 11
./run_perft.sh englishCheckers-split 11
./run_perft.sh go 3
./run_perft.sh gomoku 4
./run_perft.sh hex 4
./run_perft.sh hex_9x9 4
./run_perft.sh internationalCheckers 7
./run_perft.sh paperSoccer 6
./run_perft.sh reversi 10
./run_perft.sh theMillGame 6
./run_perft.sh ticTacToe 10
