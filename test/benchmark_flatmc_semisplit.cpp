#include <iostream>
#include <chrono>
#include "rbg_random_generator.hpp"
#include "reasoner.hpp"

typedef unsigned int uint;
typedef unsigned long ulong;
constexpr int KEEPER = 0;
constexpr uint MAX_SEMIDEPTH = 100;

RBGRandomGenerator random_generator(1);

uint semilength;
ulong states_count = 0, semistates_count = 0;
ulong goals_avg[reasoner::NUMBER_OF_PLAYERS] = {};
reasoner::resettable_bitarray_stack cache;
reasoner::game_state initial_state;
std::vector<reasoner::semimove> legal_semimoves[MAX_SEMIDEPTH];

void count_terminal(const reasoner::game_state &state){
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        goals_avg[i] += state.get_player_score(i);
}

reasoner::revert_information apply_random_semimove_from_given(reasoner::game_state &state, std::vector<reasoner::semimove> &semimoves){
    uint chosen_semimove = random_generator.uniform_choice(semimoves.size());
    reasoner::revert_information ri = state.apply_semimove_with_revert(semimoves[chosen_semimove]);
    semimoves[chosen_semimove] = semimoves.back();
    semimoves.pop_back();
    return ri;
}

std::vector<reasoner::semimove>& fill_semimoves_table(reasoner::game_state &state, uint semidepth){
    std::vector<reasoner::semimove>& semimoves = legal_semimoves[semidepth];
    state.get_all_semimoves(cache, semimoves, SEMILENGTH);
    return semimoves;
}

bool apply_random_move_exhaustive(reasoner::game_state &state, uint semidepth){
    std::vector<reasoner::semimove>& semimoves = fill_semimoves_table(state, semidepth);
    semidepth++;
    while(not semimoves.empty()){
        auto ri = apply_random_semimove_from_given(state, semimoves);
        semistates_count++;
        if(state.is_nodal())
            return true;
        if(apply_random_move_exhaustive(state, semidepth))
            return true;
        state.revert(ri);
    }
    return false;
}


void random_simulation(){
    reasoner::game_state state = initial_state;
    while(true){
        if(not apply_random_move_exhaustive(state, 0)){
            count_terminal(state);
            return;
        }
        states_count++;
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
        std::cout << "Wrong arguments. Exitting..." << std::endl;
        return 1;
    }
    std::cout << "Random generator: " << RBG_RANDOM_GENERATOR << std::endl;
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
    std::cout << "semilength: " << SEMILENGTH << std::endl;
    std::cout << "time: " << ms << " ms" << std::endl;
    std::cout << "number of playouts: " << simulations_count << " (" << std::fixed << count_per_sec(simulations_count, ms) << " playouts/sec)" << std::endl;
    std::cout << "number of states: " << states_count << " (" << std::fixed << count_per_sec(states_count, ms) << " states/sec)" << std::endl;
    std::cout << "number of semistates: " << semistates_count << " (" << std::fixed << count_per_sec(semistates_count, ms) << " states/sec)" << std::endl;
    std::cout << "depth: avg " << static_cast<long double>(states_count)/simulations_count << std::endl;
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        std::cout << "goal of player " << i << ": avg " << static_cast<long double>(goals_avg[i])/simulations_count << std::endl;
    return 0;
}
