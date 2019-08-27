#include <iostream>
#include <random>
#include <chrono>
#include "reasoner.hpp"

typedef unsigned int uint;
typedef unsigned long ulong;
constexpr int KEEPER = 0;

std::mt19937 random_generator(1);

ulong avg_goals[reasoner::NUMBER_OF_PLAYERS] = {};
int min_goals[reasoner::NUMBER_OF_PLAYERS] = {};
int max_goals[reasoner::NUMBER_OF_PLAYERS] = {};
ulong states_count = 0, moves_count = 0, depths_sum = 0;
uint min_depth = std::numeric_limits<uint>::max();
uint max_depth = std::numeric_limits<uint>::min();
reasoner::resettable_bitarray_stack cache;
reasoner::game_state initial_state;
std::vector<reasoner::move> legal_moves;

void initialize_goals_arrays(void){
    for(uint i=0;i<reasoner::NUMBER_OF_PLAYERS;++i){
        min_goals[i] = std::numeric_limits<int>::max();
        max_goals[i] = std::numeric_limits<int>::min();
    }
}

void count_terminal(const reasoner::game_state state, uint depth){
    depths_sum += depth;
    if(depth < min_depth)
        min_depth = depth;
    if(depth > max_depth)
        max_depth = depth;
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i){
        avg_goals[i] += state.get_player_score(i);
        if(state.get_player_score(i) < min_goals[i])
            min_goals[i] = state.get_player_score(i);
        if(state.get_player_score(i) > max_goals[i])
            max_goals[i] = state.get_player_score(i);
    }
}

void random_simulation(){
    reasoner::game_state state = initial_state;
    uint depth = 0;
    while(true){
        states_count++;
        state.get_all_moves(cache, legal_moves);
        if(legal_moves.empty()){
            count_terminal(state, depth);
            return;
        }
        else{
            depth++;
            moves_count += legal_moves.size();
            std::uniform_int_distribution<uint> distribution(0,legal_moves.size()-1);
            uint chosen_move = distribution(random_generator);
            state.apply_move(legal_moves[chosen_move]);
        }
        while(state.get_current_player() == KEEPER){
            auto any_move = state.apply_any_move(cache);
            if(not any_move){
                count_terminal(state, depth);
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
    initialize_goals_arrays();
    while(initial_state.get_current_player() == KEEPER){
        auto any_move = initial_state.apply_any_move(cache);
        if(not any_move)
            return 2;
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
    std::cout << "depth: avg " << static_cast<long double>(depths_sum)/simulations_count << " min " << min_depth << " max " << max_depth << std::endl;
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        std::cout << "goal of player " << i << ": avg " << static_cast<long double>(avg_goals[i])/simulations_count << " min " << min_goals[i] << " max " << max_goals[i] << std::endl;
    return 0;
}
