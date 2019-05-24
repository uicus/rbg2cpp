#include <iostream>
#include <random>
#include <chrono>
#include "reasoner.hpp"

typedef unsigned int uint;
constexpr int KEEPER = 0;

std::mt19937 random_generator(1);

uint avg_goals[reasoner::NUMBER_OF_PLAYERS] = {};
uint states_count = 0;
reasoner::resettable_bitarray_stack cache;
reasoner::game_state initial_state;

void random_simulation(){
    reasoner::game_state state = initial_state;
    while(true){
        std::vector<reasoner::move> legal_moves;
        if(state.get_current_player() == KEEPER){
            auto any_move = state.get_any_move(cache);
            if(any_move)
                state.apply_move(*any_move);
            else{
                for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
                    avg_goals[i] += state.get_player_score(i);
                return;
            }
        }
        else{
            states_count++;
            auto legal_moves = state.get_all_moves(cache);
            if(legal_moves.empty()){
                for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
                    avg_goals[i] += state.get_player_score(i);
                return;
            }
            else{
                std::uniform_int_distribution<> random_distribution(0,legal_moves.size()-1);
                uint chosen_move = random_distribution(random_generator);
                state.apply_move(legal_moves[chosen_move]);
            }
        }
    }
}

inline double count_per_sec(unsigned long count, unsigned long ms) {return static_cast<double>(count)/static_cast<double>(ms)*1000.0;}

int main(int argv, char** argc){
    if(argv != 2){
        std::cout << "Number of simulations unspecified. Exitting..." << std::endl;
        return 1;
    }
    while(initial_state.get_current_player() == KEEPER){
        auto any_move = initial_state.get_any_move(cache);
        if(any_move)
            initial_state.apply_move(*any_move);
    }
    unsigned long number_of_simulations = std::stoi(argc[1]);
    std::chrono::steady_clock::time_point start_time(std::chrono::steady_clock::now());
    for(unsigned long i = 0; i < number_of_simulations; ++i)
        random_simulation();
    std::chrono::steady_clock::time_point end_time(std::chrono::steady_clock::now());

    unsigned long ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
    std::cout << "time: " << ms << std::endl;
    std::cout << "number of plays: " << number_of_simulations << " (" << std::fixed << count_per_sec(number_of_simulations, ms) << ")" << std::endl;
    std::cout << "number of states: " << states_count << " (" << std::fixed << count_per_sec(states_count, ms) << " states/sec)" << std::endl;
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        std::cout << "average goal of player " << i << ": " << avg_goals[i]/number_of_simulations << std::endl;
    return 0;
}
