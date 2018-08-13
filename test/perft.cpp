#include <iostream>
#include "reasoner.hpp"

typedef unsigned int uint;
constexpr int KEEPER = 0;
constexpr uint DEPTH = 3;
constexpr uint CACHE_SIZE = DEPTH*4; // not enough for some games

reasoner::resettable_bitarray_stack cache[CACHE_SIZE];

uint perft_state_at_depth(reasoner::game_state& state, uint depth, uint cache_depth){
    if(depth == 0 and state.get_current_player() != KEEPER)
        return 1;
    else{
        if(state.get_current_player() == KEEPER){
            if(CACHE_SIZE <= cache_depth)
                std::cout<<"Too small cache! Test is going to crash..."<<std::endl;
            reasoner::next_states_iterator it(state, cache[cache_depth]);
            if(it.next()){
                uint result = perft_state_at_depth(state,depth,cache_depth+1);
                it.reset();
                return result;
            }
            else{
                return 0;
            }
        }
        else{
            if(CACHE_SIZE <= cache_depth)
                std::cout<<"Too small cache! Test is going to crash..."<<std::endl;
            std::vector<reasoner::move> legal_moves;
            reasoner::next_states_iterator it(state, cache[cache_depth]);
            uint result = 0;
            while(it.next()){
                legal_moves.push_back(it.get_move());
                result += perft_state_at_depth(state,depth-1,cache_depth+1);
            }
            return result;
        }
    }
}

uint perft(void){
    reasoner::game_state state;
    return perft_state_at_depth(state,DEPTH,0);
}

int main() {
    std::cout<<"Perft: "<<perft()<<std::endl;
    return 0;
}
