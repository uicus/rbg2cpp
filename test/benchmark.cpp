#include <iostream>
#include <chrono>
#include <optional>
#include "rbg_random_generator.hpp"
#include "reasoner.hpp"

typedef unsigned int uint;
typedef unsigned long ulong;
constexpr int KEEPER = 0;

RBGRandomGenerator random_generator(1);

ulong states_count = 0;
ulong goals_avg[reasoner::NUMBER_OF_PLAYERS] = {};
reasoner::resettable_bitarray_stack cache;
reasoner::game_state initial_state;

void count_terminal(const reasoner::game_state &state){
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        goals_avg[i] += state.get_player_score(i);
}

std::optional<uint> try_to_choose_random_from_monotonics(std::vector<reasoner::move>& monotonic_moves, const reasoner::game_state& state){
    uint chosen_move = random_generator.uniform_choice(monotonic_moves.size());
    if(state.is_legal(monotonic_moves[chosen_move]))
        return chosen_move;
    else{
        monotonic_moves[chosen_move] = monotonic_moves.back();
        monotonic_moves.pop_back();
        return std::nullopt;
    }
}

std::optional<uint> choose_random_from_monotonics_or_leave_empty(std::vector<reasoner::move>& monotonic_moves, const reasoner::game_state& state){
    while(not monotonic_moves.empty()){
        if(auto potential_result = try_to_choose_random_from_monotonics(monotonic_moves, state))
            return potential_result;
    }
    return std::nullopt;
}

template<int MONOTONIC_CLASSES>
void random_simulation_with_monotonic_moves(){
    static std::vector<reasoner::move> legal_moves;
    static std::vector<reasoner::move> monotonic_moves[MONOTONIC_CLASSES];
    for(uint i=0;i<MONOTONIC_CLASSES;++i)
        monotonic_moves[i].clear();
    reasoner::game_state state = initial_state;
    while(true){
        auto monotonicity_class = state.get_monotonicity_class();
        if(monotonicity_class >= 0){
            if(monotonic_moves[monotonicity_class].empty())
                state.get_all_moves(cache, monotonic_moves[monotonicity_class]);
            if(auto chosen_move = choose_random_from_monotonics_or_leave_empty(monotonic_moves[monotonicity_class], state)){
                states_count++;
                state.apply_move(monotonic_moves[monotonicity_class][*chosen_move]);
            }
            else{
                count_terminal(state);
                return;
            }
        }
        else{
            state.get_all_moves(cache, legal_moves);
            if(legal_moves.empty()){
                count_terminal(state);
                return;
            }
            else{
                states_count++;
                uint chosen_move = random_generator.uniform_choice(legal_moves.size());
                state.apply_move(legal_moves[chosen_move]);
            }
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

void random_simulation(){
    static std::vector<reasoner::move> legal_moves;
    reasoner::game_state state = initial_state;
    while(true){
        state.get_all_moves(cache, legal_moves);
        if(legal_moves.empty()){
            count_terminal(state);
            return;
        }
        else{
            states_count++;
            uint chosen_move = random_generator.uniform_choice(legal_moves.size());
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
        std::cout << "Benchmark time unspecified. Exitting..." << std::endl;
        return 1;
    }
    std::cout << "Random generator: " << RBG_RANDOM_GENERATOR << std::endl;
    std::cout << "Monotonic classes: " << reasoner::MONOTONIC_CLASSES << std::endl;
    while(initial_state.get_current_player() == KEEPER){
        auto any_move = initial_state.apply_any_move(cache);
        if(not any_move)
            return 2;
    }
    std::chrono::duration simulation_duration = std::chrono::milliseconds(std::stoi(argc[1]));

    ulong simulations_count = 0;
    std::chrono::high_resolution_clock::time_point end_time;
    std::chrono::high_resolution_clock::time_point start_time(std::chrono::high_resolution_clock::now());
    std::chrono::high_resolution_clock::time_point planned_end_time = start_time + simulation_duration;
    while(true){
        simulations_count++;
        //if constexpr (reasoner::MONOTONIC_CLASSES > 0)
        //    random_simulation_with_monotonic_moves<reasoner::MONOTONIC_CLASSES>();
        //else
            random_simulation();
        end_time = std::chrono::high_resolution_clock::now();
        if(end_time >= planned_end_time)
            break;
	}

    ulong ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
    std::cout << "time: " << ms << " ms" << std::endl;
    std::cout << "number of playouts: " << simulations_count << " (" << std::fixed << count_per_sec(simulations_count, ms) << " playouts/sec)" << std::endl;
    std::cout << "number of states: " << states_count << " (" << std::fixed << count_per_sec(states_count, ms) << " states/sec)" << std::endl;
    std::cout << "depth: avg " << static_cast<long double>(states_count)/simulations_count << std::endl;
    for(uint i=1;i<reasoner::NUMBER_OF_PLAYERS;++i)
        std::cout << "goal of player " << i << ": avg " << static_cast<long double>(goals_avg[i])/simulations_count << std::endl;
    return 0;
}
