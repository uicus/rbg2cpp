#ifndef TRANSITION_DATA
#define TRANSITION_DATA

#include"types.hpp"
#include<map>
#include<vector>
#include<string>

namespace rbg_parser{
    class token;
    class declarations;
}

class shift_table;
class precomputed_pattern;
class cpp_container;
class state;
class cache_checks_container;
class compiler_options;

enum kind_of_transition{
    all_getter,
    any_getter,
    inside_pattern,
};

enum mode{
    semisplit_off,
    semisplit_actions,
    semisplit_dotsplit
};

struct static_transition_data{
    const compiler_options& opts;
    const mode semisplit;
    const std::map<rbg_parser::token, uint>& pieces_to_id;
    const std::map<rbg_parser::token, uint>& edges_to_id;
    const std::map<rbg_parser::token, uint>& variables_to_id;
    const rbg_parser::declarations& decl;
    const std::vector<shift_table>& shift_tables;
    const std::vector<precomputed_pattern>& precomputed_patterns;
    const cache_checks_container& ccc;
    bool uses_pieces_in_arithmetics;
    bool injective_board;
    std::string return_type;
    std::string name_prefix;
    std::string success_finish;
    std::string failure_finish;
    std::string cache_level_getter;
    std::string cache_level_pusher;
    std::string cache_level_reverter;
    std::string cache_setter;
    std::string cache_set_getter;
    kind_of_transition kind;
    uint pattern_index = 0;
    static_transition_data(
        const compiler_options& opts,
        const mode& semisplit,
        const std::map<rbg_parser::token, uint>& pieces_to_id,
        const std::map<rbg_parser::token, uint>& edges_to_id,
        const std::map<rbg_parser::token, uint>& variables_to_id,
        const rbg_parser::declarations& decl,
        const std::vector<shift_table>& shift_tables,
        const std::vector<precomputed_pattern>& precomputed_patterns,
        const cache_checks_container& ccc,
        bool uses_pieces_in_arithmetics,
        bool injective_board,
        const std::string& name_prefix,
        kind_of_transition kind,
        uint pattern_index=0);
};

enum application_type{
    board_change,
    variable_change,
};

struct application{
    application_type type;
    uint additional_info;
};

class dynamic_transition_data{
        const static_transition_data& static_data;
        std::vector<application> reverting_stack;
        bool encountered_board_change;
        bool encountered_variable_change;
        bool encountered_any_change;
        bool should_check_cell_correctness;
        bool pending_modifier;
        bool has_saved_cache_level;
        bool encountered_custom_split_point;
        int split_point_action_index;
        int next_player;
        std::string from_state;
        int last_state_to_check;
        int branching_shift_table_to_handle;
        void revert_board_change(cpp_container& output, uint piece_id, uint stack_position)const;
        void revert_variable_change(cpp_container& output, uint variable_id, uint stack_position)const;
        void print_modifiers_applications_revert(cpp_container& output)const;
        void print_cache_level_revert(cpp_container& output)const;
        void print_modifiers_list_revert(cpp_container& output)const;
        void insert_unended_reverting_sequence(cpp_container& output)const;
        bool should_use_cache(void)const;
    public:
        dynamic_transition_data(const static_transition_data& static_data, uint from_state);
        const std::string& get_start_state(void)const;
        std::string get_current_state(void)const;
        void save_board_change_for_later_revert(cpp_container& output, uint piece_id);
        void save_variable_change_for_later_revert(cpp_container& output, uint variable_id);
        void insert_move_size_check(cpp_container& output, uint state_index);
        void push_any_change_on_modifiers_list(cpp_container& output, const std::string& index, const std::string& cell);
        void visit_custom_split_point(int action_index);
        void insert_reverting_sequence_after_fail(cpp_container& output)const;
        void insert_reverting_sequence_after_success(cpp_container& output)const;
        void handle_waiting_modifier(cpp_container& output);
        void handle_cell_check(cpp_container& output);
        void queue_cell_check(void);
        void set_next_player(uint next);
        bool is_ready_to_report(void)const;
        int get_next_player(void)const;
        void finallize(cpp_container& output);
        void queue_state_to_check_visited(uint state_index);
        bool should_check_for_visited(void)const;
        void visit_node(
            cpp_container& output,
            const std::string& cell="cell",
            bool custom_fail_instruction=false,
            const std::string& fail_instruction="");
        void queue_branching_shift_table(uint index);
        bool should_handle_branching_shift_table(void)const;
        void handle_branching_shift_table(cpp_container& output, const state& state_at_end, uint state_index);
        void handle_standard_transition_end(cpp_container& output, const state& state_at_end, uint state_index);
        bool can_handle_further_labels(void)const;
        void clear_queue_checks(void);
};


#endif
