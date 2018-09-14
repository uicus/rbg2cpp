#ifndef AUTOMATON
#define AUTOMATON

#include<vector>

#include"types.hpp"
#include"state.hpp"

class cpp_container;
class compiler_options;
class shift_table;
class precomputed_pattern;
class actions_compiler;
struct static_transition_data;

namespace rbg_parser{
    class game_move;
    class graph;
}

class automaton{
        std::vector<state> local_register;
        uint start_state;
        uint accept_state;
        std::pair<uint,uint> place_side_by_side(automaton&& rhs);
        std::pair<uint,uint> prepare_new_endpoints(void);
        void set_endpoints(const std::pair<uint,uint>& new_endpoints);
        void mark_connections_to_reachable_states(
            uint source_cell,
            const rbg_parser::graph& board,
            shift_table& table_to_modify,
            const std::vector<precomputed_pattern>& pps)const;
    public:
        void concat_automaton(automaton&& concatee);
        void starify_automaton(void);
        uint get_start_state(void);
        uint get_size(void);
        void print_transition_functions(
            cpp_container& output,
            const static_transition_data& static_data)const;
        void print_transition_table(
            cpp_container& output,
            const std::string& table_name,
            const std::string& functions_prefix,
            const std::string& return_type)const;
        void mark_end_as_outgoing_usable(void);
        void mark_start_as_outgoing_usable(void);
        void mark_states_as_double_reachable(const std::vector<shift_table>& shift_tables);
        shift_table generate_shift_table(
            const rbg_parser::graph& board,
            const std::vector<precomputed_pattern>& pps)const;
        void print_recursive_calls_for_pattern_in_start_state(
            cpp_container& output,
            const static_transition_data& static_data)const;
        friend automaton sum_of_automatons(std::vector<automaton>&& elements);
        friend automaton concatenation_of_automatons(std::vector<automaton>&& elements);
        friend automaton edge_automaton(const std::vector<label>& label_list);
};

automaton sum_of_automatons(std::vector<automaton>&& elements);
automaton concatenation_of_automatons(std::vector<automaton>&& elements);
automaton edge_automaton(const std::vector<label>& label_list);

#endif
