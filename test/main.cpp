// *******************************************************************************************************************
#include <iostream>
#include <random>
#include <chrono>
#include "reasoner.hpp"

typedef unsigned int uint;
constexpr uint NUMBER_OF_SIMULATIONS = 10;
constexpr int KEEPER = 0;

std::mt19937 random_generator(1);

uint avg_goals[reasoner::NUMBER_OF_PLAYERS] = {};
uint states_count = 0;

void random_simulation() {
    reasoner::game_state state;
    reasoner::resettable_bitarray_stack cache;
    while(true){
        uint legal_moves = 0;
        if(state.get_current_player() == KEEPER){
            reasoner::next_states_iterator it(state, cache);
            if(it.next()){
                continue;
            }
        }
        else{
            states_count++;
            reasoner::next_states_iterator it(state, cache);
            while (it.next()){
                legal_moves++;
            }
        }

        if(legal_moves == 0){
            for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
                avg_goals[i] += state.get_player_score(i);
            return;
        }

        std::uniform_int_distribution<> random_distribution(1,legal_moves);
        uint chosen_move = random_distribution(random_generator);
        reasoner::next_states_iterator it(state, cache);
        for(uint i=0;i<chosen_move;++i)
            it.next();
    }
}

int main() {
    std::chrono::steady_clock::time_point start_time(std::chrono::steady_clock::now());
    for(uint i=0;i<NUMBER_OF_SIMULATIONS;++i){
        std::cout << "Simulation number: " << i << std::endl;
        random_simulation();
    }
    std::chrono::steady_clock::time_point end_time(std::chrono::steady_clock::now());

    uint ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
    std::cout << "performed " << NUMBER_OF_SIMULATIONS << " in " << ms << " ms" << std::endl;
    std::cout << "average time " << ms/NUMBER_OF_SIMULATIONS << " ms" << std::endl;
    std::cout << "visited " << states_count << " states" << std::endl;
    std::cout << static_cast<double>(states_count)/static_cast<double>(ms)*1000.0 << " states per sec" << std::endl;
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        std::cout << "average goal of player " << i << ": " << avg_goals[i]/NUMBER_OF_SIMULATIONS << std::endl;
    return 0;
}
