#include <iostream>
#include <chrono>
#include <boost/container/small_vector.hpp>
#include "reasoner.hpp"

typedef unsigned int uint;
constexpr int KEEPER = 0;
constexpr uint DEPTH = 4;

reasoner::resettable_bitarray_stack cache;
uint states_count;

// crude solution, just for checking correctness
uint perft_state_at_depth(const reasoner::game_state& state, uint depth){
    if(depth == 0 and state.get_current_player() != KEEPER){
        ++states_count;
        return 1;
    }
    else{
        auto copy_state = state;
        if(state.get_current_player() == KEEPER){
            auto any_move = copy_state.get_any_move(cache);
            if(any_move.first){
                copy_state.apply_move(any_move.second);
                return perft_state_at_depth(copy_state, depth);
            }
            else
                return 0;
        }
        else{
            ++states_count;
            auto legal_moves = copy_state.get_all_moves(cache);
            uint result = 0;
            for(const auto& el: legal_moves){
                auto temp_state = copy_state;
                temp_state.apply_move(el);
                result += perft_state_at_depth(temp_state,depth-1);
            }
            return result;
        }
    }
}

uint perft(void){
    reasoner::game_state state;
    return perft_state_at_depth(state,DEPTH);
}

int main() {
 std::chrono::steady_clock::time_point start_time(std::chrono::steady_clock::now());
    auto p = perft();
    std::chrono::steady_clock::time_point end_time(std::chrono::steady_clock::now());

    uint ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
    std::cout<<"perft: "<<p<<std::endl;
    std::cout<<"took: "<<ms<<" ms"<<std::endl;
    std::cout<<"visited: "<<states_count<<" states"<<std::endl;
    std::cout << std::fixed << static_cast<double>(states_count)/static_cast<double>(ms)*1000.0 << " states/sec" << std::endl;
    return 0;
}
