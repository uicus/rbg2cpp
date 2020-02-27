#ifndef MONOTONIC_MOVE
#define MONOTONIC_MOVE

#include"types.hpp"
#include"token.hpp"
#include<optional>
#include<set>

namespace rbg_parser{
    class game_move;
}

class shift_table;
class cpp_container;
class static_transition_data;

struct monotonic_move{
    uint start_state;
    const shift_table* cell_choice;
    std::optional<std::set<rbg_parser::token>> pieces_choice;
    std::vector<int> end_action_indices;
    void print_legality_check(cpp_container& output, const static_transition_data& static_data)const;
};

#endif
