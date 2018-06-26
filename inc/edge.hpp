#ifndef EDGE
#define EDGE

#include<vector>
#include"types.hpp"

namespace rbg_parser{
    class game_move;
}

class edge{
        uint local_register_endpoint_index;
        std::vector<const rbg_parser::game_move*> label_list;
    public:
        edge(uint local_register_endpoint_index);
        void add_another_action(const rbg_parser::game_move* action);
        void shift(uint shift_value);
        void inform_abut_state_deletion(uint deleted_index);
};

#endif
