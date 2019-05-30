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

namespace rbg_parser{
    class game_move;
    class graph;
}

class state{
        std::vector<edge> next_states;
        int state_to_check_before_next_alternatives = -1;
        bool move_ender = false;
        std::vector<uint> states_to_mark_after_reaching;
        bool outgoing_edges_needed;
        bool doubly_reachable;
    public:
        void inform_about_being_appended(uint shift_value);
        void inform_about_state_deletion(uint deleted_index);
        void set_state_to_see_before_continuing(int state_index);
        void add_information_about_state_to_see(std::map<uint,uint>& states_to_bool_array)const;
        void absorb(state&& rhs);
        void connect_with_state(uint index_in_local_register, const std::vector<label>& label_list=std::vector<label>());
        void print_transition_functions(
            uint from_state,
            cpp_container& output,
            const static_transition_data& static_data,
            const std::vector<state>& local_register)const;
        void print_outgoing_any_transitions(uint from_state, cpp_container& output, const std::string& functions_prefix)const;
        void print_outgoing_all_transitions(uint from_state, cpp_container& output, const std::string& functions_prefix)const;
        void notify_endpoints_about_being_reachable(std::vector<uint>& reachability, const std::vector<shift_table>& shift_tables)const;
        void mark_as_doubly_reachable(void);
        bool can_be_checked_for_visit(void)const;
        void mark_explicitly_as_transition_start(void);
        void mark_as_move_ender(void);
        const edge& get_only_exit(void)const;
        bool is_dead_end(void)const;
        bool is_no_choicer(void)const;
        void run_dfs_to_get_states_to_mark(
            uint from_state,
            std::vector<uint>& states_to_mark_if_end,
            std::vector<bool>& visited_states,
            std::vector<state>& local_register);
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
            const std::string& cell="current_cell")const;
        void print_marking_for_prioritized_sum(cpp_container& output, const static_transition_data& static_data)const;
        void print_indices_to_actions_correspondence(
            cpp_container& output,
            const static_transition_data& static_data)const;
};

#endif
