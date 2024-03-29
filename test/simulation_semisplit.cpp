#include <iostream>
#include <chrono>
#include "rbg_random_generator.hpp"
#include "reasoner.hpp"

typedef unsigned int uint;
typedef unsigned long ulong;
constexpr int KEEPER = 0;

constexpr uint MAX_SEMIDEPTH = 100;

RBGRandomGenerator random_generator(1);

ulong goals_avg[reasoner::NUMBER_OF_PLAYERS] = {};
int goals_min[reasoner::NUMBER_OF_PLAYERS] = {};
int goals_max[reasoner::NUMBER_OF_PLAYERS] = {};

ulong states_count = 0, semistates_count = 0, dead_semistates_count = 0, semimoves_count = 0;
ulong semidepth_sum = 0;
uint semidepth_min = std::numeric_limits<uint>::max();
uint semidepth_max = std::numeric_limits<uint>::min();

uint depth_min = std::numeric_limits<uint>::max();
uint depth_max = std::numeric_limits<uint>::min();

reasoner::resettable_bitarray_stack cache;
reasoner::game_state initial_state;
std::vector<reasoner::action_representation> legal_semimoves[MAX_SEMIDEPTH];

void initialize_goals_arrays(void){
    for(uint i=0;i<reasoner::NUMBER_OF_PLAYERS;++i){
        goals_min[i] = std::numeric_limits<int>::max();
        goals_max[i] = std::numeric_limits<int>::min();
    }
}

void count_terminal(const reasoner::game_state state, uint depth){
    states_count += depth;
    if(depth < depth_min)
        depth_min = depth;
    if(depth > depth_max)
        depth_max = depth;
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i){
        goals_avg[i] += state.get_player_score(i);
        if(state.get_player_score(i) < goals_min[i])
            goals_min[i] = state.get_player_score(i);
        if(state.get_player_score(i) > goals_max[i])
            goals_max[i] = state.get_player_score(i);
    }
}

void count_semiterminal(const uint semidepth){
    semidepth_sum += semidepth;
    if(semidepth < semidepth_min)
        semidepth_min = semidepth;
    if(semidepth > semidepth_max)
        semidepth_max = semidepth;
}

std::vector<reasoner::action_representation>& fill_semimoves_table(reasoner::game_state &state, uint semidepth){
    std::vector<reasoner::action_representation>& semimoves = legal_semimoves[semidepth];
    state.get_all_actions(cache, semimoves);
    semimoves_count += semimoves.size();
    return semimoves;
}

bool apply_random_move_exhaustive(reasoner::game_state &state, uint semidepth){
    std::vector<reasoner::action_representation>& semimoves = fill_semimoves_table(state, semidepth);
    semidepth++;
    while(not semimoves.empty()){
        semistates_count++;
        uint chosen_semimove = random_generator.uniform_choice(semimoves.size());
        auto ri = state.apply_action_with_revert(semimoves[chosen_semimove]);
        if(state.is_nodal()){
            count_semiterminal(semidepth);
            return true;
        }
        if(apply_random_move_exhaustive(state, semidepth))
            return true;
        state.revert(ri);
        dead_semistates_count++;
        semimoves[chosen_semimove] = semimoves.back();
        semimoves.pop_back();
    }
    return false;
}

void random_simulation(){
    reasoner::game_state state = initial_state;
    uint depth = 0;
    while(true){
        if(not apply_random_move_exhaustive(state, 0)){
            count_terminal(state, depth);
            return;
        }
        depth++;
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
        std::cout << "Wrong arguments. Exitting..." << std::endl;
        return 1;
    }
    std::cout << "Random generator: " << RBG_RANDOM_GENERATOR << std::endl;
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
    std::cout << "number of playouts: " << simulations_count << " (" << std::fixed << count_per_sec(simulations_count, ms) << " playouts/sec)" << std::endl;
    std::cout << "number of states: " << states_count << " (" << std::fixed << count_per_sec(states_count, ms) << " states/sec)" << std::endl;
    std::cout << "number of semistates: " << semistates_count << " (" << std::fixed << count_per_sec(semistates_count, ms) << " semistates/sec)" << std::endl;
    std::cout << "number of semimoves: " << semimoves_count << " (" << std::fixed << count_per_sec(semimoves_count, ms) << " semimoves/sec)" << std::endl;
    std::cout << "branching factor (semimoves): avg " << static_cast<long double>(semimoves_count)/semistates_count << std::endl;
    std::cout << "depth: avg " << static_cast<long double>(states_count)/simulations_count << " min " << depth_min << " max " << depth_max << std::endl;
    std::cout << "depth (semimoves): avg " << static_cast<long double>(semistates_count)/simulations_count << std::endl;
    std::cout << "depth (semimoves, no dead): avg " << static_cast<long double>(semistates_count-dead_semistates_count)/simulations_count << std::endl;
    std::cout << "semidepth: avg " << static_cast<long double>(semidepth_sum)/states_count << " min " << semidepth_min << " max " << semidepth_max << std::endl;
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        std::cout << "goal of player " << i << ": avg " << static_cast<long double>(goals_avg[i])/simulations_count << " min " << goals_min[i] << " max " << goals_max[i] << std::endl;
    return 0;
}
