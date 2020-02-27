#ifndef AUTOMATON
#define AUTOMATON

#include<vector>

#include"types.hpp"
#include"state.hpp"
#include"monotonic_move.hpp"

class cpp_container;
class compiler_options;
class shift_table;
class precomputed_pattern;
class actions_compiler;
struct static_transition_data;
class rules_board_automaton;

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
        void print_any_appliers_table(cpp_container& output, const std::string& functions_prefix, bool cache_used)const;
        void print_all_getters_table(cpp_container& output, const std::string& functions_prefix, bool cache_used, bool semisplit_enabled)const;
        void mark_end_as_keeper_move_start(void);
        void mark_end_as_player_move_start(void);
        void mark_end_as_outgoing_usable(void);
        void mark_start_as_rules_beginning(void);
        void mark_states_as_double_reachable(const std::vector<shift_table>& shift_tables);
        shift_table generate_shift_table(
            const rbg_parser::graph& board,
            const std::vector<precomputed_pattern>& pps)const;
        void print_recursive_calls_for_pattern_in_start_state(
            cpp_container& output,
            const static_transition_data& static_data)const;
        void print_indices_to_actions_correspondence(
            cpp_container& output,
            const static_transition_data& static_data,
            bool generate_revert=false)const;
        void print_final_action_effects(cpp_container& output)const;
        rules_board_automaton generate_rules_board_automaton(
            const std::vector<shift_table>& shift_tables,
            const std::vector<precomputed_pattern>& precomputed_patterns,
            const std::vector<std::vector<uint>>& board_structure,
            const std::map<rbg_parser::token, uint>& edges_to_id)const;
        void print_is_nodal_function(cpp_container& output)const;
        std::vector<monotonic_move> get_monotonics(const std::vector<shift_table>& shift_tables)const;
        friend automaton sum_of_automatons(std::vector<automaton>&& elements);
        friend automaton prioritized_sum_of_automatons(std::vector<automaton>&& elements);
        friend automaton concatenation_of_automatons(std::vector<automaton>&& elements);
        friend automaton edge_automaton(const std::vector<label>& label_list);
};

automaton sum_of_automatons(std::vector<automaton>&& elements);
automaton concatenation_of_automatons(std::vector<automaton>&& elements);
automaton edge_automaton(const std::vector<label>& label_list);

#endif
