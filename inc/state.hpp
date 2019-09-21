#ifndef STATE
#define STATE

#include<vector>
#include<set>
#include<map>

#include"types.hpp"
#include"edge.hpp"

class cpp_container;
class compiler_options;
class precomputed_pattern;
class actions_compiler;
struct static_transition_data;
class rules_board_automaton;

namespace rbg_parser{
    class game_move;
    class graph;
}

enum state_user_type{
    none,
    player_start,
    keeper_start
};

class state{
        std::vector<edge> next_states;
        bool outgoing_edges_needed;
        bool doubly_reachable;
        state_user_type state_user = none;
    public:
        void inform_about_being_appended(uint shift_value);
        void inform_about_state_deletion(uint deleted_index);
        void absorb(state&& rhs);
        void connect_with_state(uint index_in_local_register, const std::vector<label>& label_list=std::vector<label>());
        void print_transition_functions(
            uint from_state,
            cpp_container& output,
            const static_transition_data& static_data,
            const std::vector<state>& local_register)const;
        void print_outgoing_any_transitions(uint from_state, cpp_container& output, const std::string& functions_prefix, bool cache_used)const;
        void print_outgoing_all_transitions(uint from_state, cpp_container& output, const std::string& functions_prefix, bool cache_used, bool semisplit_enabled)const;
        void notify_endpoints_about_being_reachable(std::vector<uint>& reachability, const std::vector<shift_table>& shift_tables)const;
        void mark_as_keeper_move_beginning(void);
        void mark_as_player_move_beginning(void);
        void mark_as_doubly_reachable(void);
        bool can_be_checked_for_visit(void)const;
        void mark_explicitly_as_transition_start(void);
        const edge& get_only_exit(void)const;
        bool is_dead_end(void)const;
        bool is_no_choicer(void)const;
        bool is_full_state(void)const;
        void push_next_states_to_shift_tables_dfs_stack(
            uint current_cell,
            const rbg_parser::graph& board,
            std::vector<std::pair<uint,uint>>& dfs_stack,
            const std::vector<precomputed_pattern>& pps)const;
        void print_recursive_calls(
            uint from_state,
            cpp_container& output,
            const static_transition_data& static_data,
            dynamic_transition_data& dynamic_data,
            const std::string& cell="cell")const;
        void print_indices_to_actions_correspondence(
            cpp_container& output,
            const static_transition_data& static_data)const;
        void print_final_action_effects(cpp_container& output)const;
        void add_state_to_board_automaton(
            uint own_number,
            rules_board_automaton& rba,
            const std::vector<shift_table>& shift_tables,
            const std::vector<precomputed_pattern>& precomputed_patterns,
            const std::vector<std::vector<uint>>& board_structure,
            const std::map<rbg_parser::token, uint>& edges_to_id)const;
        void print_last_edge_modifier_to_cell_change_correspondence(
            cpp_container& output,
            const std::vector<shift_table>& shift_tables,
            const std::vector<std::vector<uint>>& board_structure,
            const std::map<rbg_parser::token, uint>& edges_to_id,
            std::map<uint, uint>& modifier_to_cell_change_table)const;
};

#endif
