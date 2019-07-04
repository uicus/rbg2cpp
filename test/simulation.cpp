#include <iostream>
#include <random>
#include <chrono>
#include "reasoner.hpp"

typedef unsigned int uint;
typedef unsigned long ulong;
constexpr int KEEPER = 0;

std::mt19937 random_generator(1);

ulong avg_goals[reasoner::NUMBER_OF_PLAYERS] = {};
ulong states_count = 0, moves_count = 0;
reasoner::resettable_bitarray_stack cache;
reasoner::game_state initial_state;

void random_simulation(){
    reasoner::game_state state = initial_state;
    std::vector<reasoner::move> legal_moves;
    while(true){
        if(state.get_current_player() == KEEPER){
            auto any_move = state.apply_any_move(cache);
            if(not any_move){
                for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
                    avg_goals[i] += state.get_player_score(i);
                return;
            }
        }
        else{
            states_count++;
            state.get_all_moves(cache, legal_moves);
            if(legal_moves.empty()){
                for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
                    avg_goals[i] += state.get_player_score(i);
                return;
            }
            else{
                moves_count += legal_moves.size();
                std::uniform_int_distribution<> random_distribution(0,legal_moves.size()-1);
                uint chosen_move = random_distribution(random_generator);
                state.apply_move(legal_moves[chosen_move]);
            }
        }
    }
}

double count_per_sec(ulong count, ulong ms){
    return static_cast<double>(count)/static_cast<double>(ms)*1000.0;
}

int main(int argv, char** argc){
    if(argv != 2){
        std::cout << "Number of simulations unspecified. Exitting..." << std::endl;
        return 1;
    }
    while(initial_state.get_current_player() == KEEPER){
        auto any_move = initial_state.apply_any_move(cache);
        if(not any_move)
            return 0;
    }
    ulong simulations_count = std::stoi(argc[1]);

    std::chrono::steady_clock::time_point start_time(std::chrono::steady_clock::now());
    for(ulong i = 0; i < simulations_count; ++i)
        random_simulation();
    std::chrono::steady_clock::time_point end_time(std::chrono::steady_clock::now());

    ulong ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
    std::cout << "time: " << ms << " ms" << std::endl;
    std::cout << "number of plays: " << simulations_count << " (" << std::fixed << count_per_sec(simulations_count, ms) << " plays/sec)" << std::endl;
    std::cout << "number of states: " << states_count << " (" << std::fixed << count_per_sec(states_count, ms) << " states/sec)" << std::endl;
    std::cout << "number of moves: " << moves_count << " (" << std::fixed << count_per_sec(moves_count, ms) << " moves/sec)" << std::endl;
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        std::cout << "average goal of player " << i << ": " << avg_goals[i]/simulations_count << std::endl;
    return 0;
}
