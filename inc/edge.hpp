#ifndef EDGE
#define EDGE

#include<vector>
#include<map>
#include"types.hpp"
#include"token.hpp"

class cpp_container;
class state;

namespace rbg_parser{
    class game_move;
    class declarations;
}

enum label_kind{
    action,
    positive_pattern,
    negative_pattern
};

struct label{
    label_kind k;
    const rbg_parser::game_move* a;
    uint automaton_index;
};

class edge{
        uint local_register_endpoint_index;
        std::vector<label> label_list;
        uint index_after_traversing;
    public:
        edge(uint local_register_endpoint_index);
        void add_another_action(const rbg_parser::game_move* a);
        void add_another_pattern_check(bool positive, uint automaton_index);
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
            const rbg_parser::declarations& decl,
            const std::vector<state>& local_register)const;
        void print_transition_function_inside_pattern(
            uint from_state,
            uint pattern_index,
            cpp_container& output,
            const std::map<rbg_parser::token, uint>& pieces_to_id,
            const std::map<rbg_parser::token, uint>& edges_to_id,
            const std::map<rbg_parser::token, uint>& variables_to_id,
            const rbg_parser::declarations& decl,
            const std::vector<state>& local_register)const;
};

#endif
