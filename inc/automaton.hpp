#ifndef AUTOMATON
#define AUTOMATON

#include<vector>

#include"types.hpp"
#include"state.hpp"

class cpp_container;
class compiler_options;

namespace rbg_parser{
    class game_move;
}

class automaton{
        std::vector<state> local_register;
        uint start_state;
        uint accept_state;
        std::pair<uint,uint> place_side_by_side(automaton&& rhs);
        std::pair<uint,uint> prepare_new_endpoints(void);
        void set_endpoints(const std::pair<uint,uint>& new_endpoints);
    public:
        void concat_automaton(automaton&& concatee);
        void starify_automaton(void);
        uint get_start_state(void);
        uint get_size(void);
        void print_transition_functions(
            cpp_container& output,
            const std::map<rbg_parser::token, uint>& pieces_to_id,
            const std::map<rbg_parser::token, uint>& edges_to_id,
            const std::map<rbg_parser::token, uint>& variables_to_id,
            const rbg_parser::declarations& decl,
            const compiler_options& opts)const;
        void print_transition_functions_inside_pattern(
            uint pattern_index,
            cpp_container& output,
            const std::map<rbg_parser::token, uint>& pieces_to_id,
            const std::map<rbg_parser::token, uint>& edges_to_id,
            const std::map<rbg_parser::token, uint>& variables_to_id,
            const rbg_parser::declarations& decl,
            const compiler_options& opts)const;
        void print_transition_table(cpp_container& output, const std::string& table_name, const std::string& functions_prefix)const;
        void mark_end_as_outgoing_usable(void);
        void mark_start_as_outgoing_usable(void);
        void mark_states_as_double_reachable(void);
        friend automaton sum_of_automatons(std::vector<automaton>&& elements);
        friend automaton concatenation_of_automatons(std::vector<automaton>&& elements);
        friend automaton edge_automaton(const std::vector<label>& label_list);
        friend automaton edge_automaton(const rbg_parser::game_move*);
        friend automaton edge_automaton(uint pattern_automaton_index, bool positive);
};

automaton sum_of_automatons(std::vector<automaton>&& elements);
automaton concatenation_of_automatons(std::vector<automaton>&& elements);
automaton edge_automaton(const std::vector<label>& label_list);

#endif
