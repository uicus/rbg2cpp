#include <iostream>
#include <chrono>
#include "reasoner.hpp"

typedef unsigned int uint;
typedef unsigned long ulong;
constexpr int KEEPER = 0;

reasoner::resettable_bitarray_stack cache;
reasoner::game_state initial_state;
ulong states_count, leaves_count;

void perft_state_at_depth(reasoner::game_state& state, uint depth){
    if(depth == 0 and state.is_nodal() and state.get_current_player() != KEEPER){
        ++states_count;
        ++leaves_count;
        return;
    }
    else{
        if(state.get_current_player() == KEEPER){
            auto temp_state = state;
            auto any_move = temp_state.apply_any_move(cache);
            if(any_move)
                return perft_state_at_depth(temp_state, depth);
            else{
                ++states_count;
                if(depth == 0)
                    ++leaves_count;
                return;
            }
        }
        else{
            if(state.is_nodal())
                ++states_count;
            std::vector<reasoner::move> legal_semimoves;
            state.get_all_semimoves(cache, legal_semimoves, 1);
            for(const auto& el: legal_semimoves){
                auto was_nodal = state.is_nodal();
                auto reverter = state.apply_move(el);
                if(was_nodal)
                    perft_state_at_depth(state,depth-1);
                else
                    perft_state_at_depth(state,depth);
                state.revert(reverter);
            }
        }
    }
}

void perft(uint depth){
    perft_state_at_depth(initial_state,depth);
}

double count_per_sec(ulong count, ulong ms){
    return static_cast<double>(count)/static_cast<double>(ms)*1000.0;
}

int main(int argv, char** argc){
    if(argv != 2){
        std::cout << "Perft depth unspecified. Exitting..." << std::endl;
        return 1;
    }
    while(initial_state.get_current_player() == KEEPER){
        auto any_move = initial_state.apply_any_move(cache);
        if(not any_move)
            return 0;
    }

    uint depth = std::stoi(argc[1]);
    std::chrono::steady_clock::time_point start_time(std::chrono::steady_clock::now());
    perft(depth);
    std::chrono::steady_clock::time_point end_time(std::chrono::steady_clock::now());

    ulong ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
    std::cout << "time: " << ms << " ms" << std::endl;
    std::cout << "perft: " << leaves_count << std::endl;
    std::cout << "number of states: " << states_count << " (" << std::fixed << count_per_sec(states_count, ms) << " states/sec)" << std::endl;
    return 0;
}
