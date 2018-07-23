#ifndef EDGE
#define EDGE

#include<vector>
#include<map>
#include"types.hpp"
#include"token.hpp"

class cpp_container;

namespace rbg_parser{
    class game_move;
    class declarations;
}

class edge{
        uint local_register_endpoint_index;
        std::vector<const rbg_parser::game_move*> label_list;
        uint index_after_traversing;
    public:
        edge(uint local_register_endpoint_index);
        void add_another_action(const rbg_parser::game_move* action);
        void set_index(uint index);
        void shift(uint shift_value);
        void inform_abut_state_deletion(uint deleted_index);
        uint get_endpoint(void)const;
        void print_transition_function(
            uint from_state,
            cpp_container& output,
            const std::map<rbg_parser::token, uint>& pieces_to_id,
            const std::map<rbg_parser::token, uint>& edges_to_id,
            const std::map<rbg_parser::token, uint>& variables_to_id,
            const rbg_parser::declarations& decl)const;
};

#endif
