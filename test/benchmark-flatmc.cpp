#include <iostream>
#include <random>
#include <chrono>
#include "reasoner.hpp"

typedef unsigned int uint;
typedef unsigned long ulong;
constexpr int KEEPER = 0;

std::mt19937 random_generator(1);

ulong states_count = 0;
ulong goals_avg[reasoner::NUMBER_OF_PLAYERS] = {};
reasoner::resettable_bitarray_stack cache;
reasoner::game_state initial_state;
std::vector<reasoner::move> legal_moves;

void count_terminal(const reasoner::game_state &state){
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        goals_avg[i] += state.get_player_score(i);
}

void random_simulation(){
    reasoner::game_state state = initial_state;
    while(true){
        states_count++;
        state.get_all_moves(cache, legal_moves);
        if(legal_moves.empty()){
            count_terminal(state);
            return;
        }
        else{
            std::uniform_int_distribution<> distribution(0,legal_moves.size()-1);
            uint chosen_move = distribution(random_generator);
            state.apply_move(legal_moves[chosen_move]);
        }
        while(state.get_current_player() == KEEPER){
            auto any_move = state.apply_any_move(cache);
            if(not any_move){
                count_terminal(state);
                return;
            }
        }
    }
}

double count_per_sec(ulong count, ulong ms){
    return static_cast<long double>(count)/ms*1000.0;
}

int main(int argv, char** argc){
    if(argv != 2){
        std::cout << "Number of simulations unspecified. Exitting..." << std::endl;
        return 1;
    }
    while(initial_state.get_current_player() == KEEPER){
        auto any_move = initial_state.apply_any_move(cache);
        if(not any_move)
            return 2;
    }
    std::chrono::duration simulation_duration = std::chrono::milliseconds(std::stoi(argc[1]));

    ulong simulations_count = 0;
    std::chrono::steady_clock::time_point end_time;
    std::chrono::steady_clock::time_point start_time(std::chrono::steady_clock::now());
    std::chrono::steady_clock::time_point planned_end_time = start_time + simulation_duration;
    while(true){
        simulations_count++;
        random_simulation();
        end_time = std::chrono::steady_clock::now();
        if(end_time >= planned_end_time)
            break;
	}

    ulong ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
    std::cout << "time: " << ms << " ms" << std::endl;
    std::cout << "number of plays: " << simulations_count << " (" << std::fixed << count_per_sec(simulations_count, ms) << " plays/sec)" << std::endl;
    std::cout << "number of states: " << states_count << " (" << std::fixed << count_per_sec(states_count, ms) << " states/sec)" << std::endl;
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        std::cout << "goal of player " << i << ": avg " << static_cast<long double>(goals_avg[i])/simulations_count << std::endl;
    return 0;
}
