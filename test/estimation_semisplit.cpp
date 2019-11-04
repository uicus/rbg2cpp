#include <iostream>
#include <random>
#include <chrono>
#include "reasoner.hpp"

typedef unsigned int uint;
typedef unsigned long ulong;
constexpr int KEEPER = 0;

constexpr uint MAX_SEMIDEPTH = 100;
constexpr uint MAX_SEMILENGTH = 32;
constexpr uint RANDOM_SEED = 1;

std::mt19937 random_generator;

uint semilength;

ulong goals_avg[reasoner::NUMBER_OF_PLAYERS];
int goals_min[reasoner::NUMBER_OF_PLAYERS];
int goals_max[reasoner::NUMBER_OF_PLAYERS];

ulong simulations_count, states_count, semistates_count, semimoves_count;
ulong semidepth_sum;
uint semidepth_min, semidepth_max, semimovelength_max;
uint depth_min, depth_max;

reasoner::resettable_bitarray_stack cache;
reasoner::game_state initial_state;
std::vector<reasoner::move> legal_semimoves[MAX_SEMIDEPTH];

void initialize(void){
	random_generator = std::mt19937(RANDOM_SEED);
	simulations_count = states_count = semistates_count = semimoves_count = semidepth_sum = semimovelength_max = 0;
    semidepth_min = depth_min = std::numeric_limits<uint>::max();
    semidepth_max = depth_max = semimovelength_max = std::numeric_limits<uint>::min();
    for(uint i=0;i<reasoner::NUMBER_OF_PLAYERS;++i){
		goals_avg[i] = 0;
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

reasoner::revert_information apply_random_semimove_from_given(reasoner::game_state &state, std::vector<reasoner::move> &semimoves){
    std::uniform_int_distribution<uint> distribution(0,semimoves.size()-1);
    uint chosen_semimove = distribution(random_generator);
    reasoner::revert_information ri = state.apply_move_with_revert(semimoves[chosen_semimove]);
    semimoves[chosen_semimove] = semimoves.back();
    semimoves.pop_back();
    return ri;
}

std::vector<reasoner::move>& fill_semimoves_table(reasoner::game_state &state, uint semidepth){
    std::vector<reasoner::move>& semimoves = legal_semimoves[semidepth];
    state.get_all_semimoves(cache, semimoves, semilength);
    semimoves_count += semimoves.size();
    return semimoves;
}

bool apply_random_move_exhaustive(reasoner::game_state &state, uint semidepth){
    std::vector<reasoner::move>& semimoves = fill_semimoves_table(state, semidepth);
    for (reasoner::move &m: semimoves) if (m.mr.size() > semimovelength_max) semimovelength_max = m.mr.size();
    semidepth++;
    while(not semimoves.empty()){
        auto ri = apply_random_semimove_from_given(state, semimoves);
        semistates_count++;
        if(state.is_nodal()){
            count_semiterminal(semidepth);
            return true;
        }
        if(apply_random_move_exhaustive(state, semidepth))
            return true;
        state.revert(ri);
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

ulong run_simulations_for_duration(const ulong ms) {
    initialize();
    std::chrono::steady_clock::time_point end_time;
    std::chrono::steady_clock::time_point start_time(std::chrono::steady_clock::now());
    std::chrono::steady_clock::time_point planned_end_time = start_time + std::chrono::milliseconds(ms);
    while(true){
        simulations_count++;
        random_simulation();
        end_time = std::chrono::steady_clock::now();
        if(end_time >= planned_end_time)
            break;
	}
	return std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
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
    ulong estimation_time_ms = std::stoi(argc[1]);

	double best_eff = 0.0;
	uint best_semilength = 1;
	for (semilength = 1; semilength <= MAX_SEMILENGTH; semilength++){
		ulong used_time = run_simulations_for_duration(estimation_time_ms);
		double eff = count_per_sec(simulations_count, used_time);
		std::cout << semilength << ": plays/sec " << eff;
		std::cout << " simulations " << simulations_count << " semimovelength_max " << semimovelength_max;
		std::cout << " davg " << static_cast<long double>(states_count)/simulations_count << " dmin " << depth_min << " dmax " << depth_max;
        std::cout << " sdavg " << static_cast<long double>(semidepth_sum)/states_count << " sdmin " << semidepth_min << " sdmax " << semidepth_max;
        std::cout << " semistates " << semistates_count << " semimoves " << semimoves_count << std::endl;
        if (best_eff < eff){
		    best_eff = eff;
		    best_semilength = semilength;
		}
		if (semimovelength_max < semilength) break;
	}
	std::cout << "Best semilength = " << best_semilength << std::endl;
	
    return 0;
}
