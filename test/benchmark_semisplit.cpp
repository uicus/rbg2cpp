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
std::vector<reasoner::action_representation> legal_semimoves[MAX_SEMIDEPTH];

void count_terminal(const reasoner::game_state &state){
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        goals_avg[i] += state.get_player_score(i);
}

std::vector<reasoner::action_representation>& fill_semimoves_table(reasoner::game_state &state, uint semidepth){
    std::vector<reasoner::action_representation>& semimoves = legal_semimoves[semidepth];
    state.get_all_semimoves(cache, semimoves);
    return semimoves;
}

bool apply_random_move_exhaustive(reasoner::game_state &state, uint semidepth=0){
    std::vector<reasoner::action_representation>& semimoves = fill_semimoves_table(state, semidepth);
    semidepth++;
    while(not semimoves.empty()){
        semistates_count++;
        uint chosen_semimove = random_generator.uniform_choice(semimoves.size());
        auto ri = state.apply_action_with_revert(semimoves[chosen_semimove]);
        if(state.is_nodal())
            return true;
        if(apply_random_move_exhaustive(state, semidepth))
            return true;
        state.revert(ri);
        semimoves[chosen_semimove] = semimoves.back();
        semimoves.pop_back();
    }
    return false;
}

std::vector<reasoner::move> moves;
reasoner::move mv;

void get_all_moves_from_semimoves(reasoner::game_state &state, uint semidepth) {
    std::vector<reasoner::action_representation>& semimoves = fill_semimoves_table(state, semidepth);
    semidepth++;
    while(not semimoves.empty()){
        semistates_count++;
        uint chosen_semimove = random_generator.uniform_choice(semimoves.size());
        if (semimoves[chosen_semimove].index > 0)
            mv.mr.emplace_back(semimoves[chosen_semimove]);
        auto ri = state.apply_action_with_revert(semimoves[chosen_semimove]);
        if(state.is_nodal())
            moves.push_back(mv); else
            get_all_moves_from_semimoves(state, semidepth);
        state.revert(ri);
        if (semimoves[chosen_semimove].index > 0)
            mv.mr.pop_back();
        semimoves[chosen_semimove] = semimoves.back();
        semimoves.pop_back();
    }
}

bool apply_random_move_exhaustive_joint(reasoner::game_state &state) {
    moves.clear();
    get_all_moves_from_semimoves(state, 0);
    if (moves.size() == 0) return false;
    uint choice = random_generator.uniform_choice(moves.size());
    state.apply_move(moves[choice]);
    return true;
}

void random_simulation(){
    reasoner::game_state state = initial_state;
    while(true){
        if(not apply_random_move_exhaustive(state)){
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
    std::cout << "time: " << ms << " ms" << std::endl;
    std::cout << "number of playouts: " << simulations_count << " (" << std::fixed << count_per_sec(simulations_count, ms) << " playouts/sec)" << std::endl;
    std::cout << "number of states: " << states_count << " (" << std::fixed << count_per_sec(states_count, ms) << " states/sec)" << std::endl;
    std::cout << "number of semistates: " << semistates_count << " (" << std::fixed << count_per_sec(semistates_count, ms) << " states/sec)" << std::endl;
    std::cout << "depth: avg " << static_cast<long double>(states_count)/simulations_count << std::endl;
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        std::cout << "goal of player " << i << ": avg " << static_cast<long double>(goals_avg[i])/simulations_count << std::endl;
    return 0;
}
