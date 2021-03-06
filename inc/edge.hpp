#ifndef EDGE
#define EDGE

#include<vector>
#include<map>
#include<tuple>
#include<string>
#include"types.hpp"
#include"token.hpp"

class cpp_container;
class state;
class actions_compiler;
class compiler_options;
class precomputed_pattern;
class shift_table;
struct static_transition_data;
class dynamic_transition_data;
class monotonicity_determiner;

namespace rbg_parser{
    class game_move;
    class declarations;
    class graph;
}

enum label_kind{
    action,
    positive_pattern,
    negative_pattern,
    s_pattern,
    s_table,
    always_true,
    always_false
};

struct label{
    label_kind k;
    const rbg_parser::game_move* a;
    uint structure_index;
};

class edge{
        uint local_register_endpoint_index;
        std::vector<label> label_list;
        void handle_labels(
            cpp_container& output,
            const static_transition_data& static_data,
            dynamic_transition_data& dynamic_data)const;
        void print_function_signature(cpp_container& output,
                                      const std::string& return_type,
                                      const std::string& name,
                                      const std::vector<std::string>& arguments)const;
    public:
        edge(uint local_register_endpoint_index, const std::vector<label>& label_list);
        void shift(uint shift_value);
        void inform_abut_state_deletion(uint deleted_index);
        uint get_endpoint(void)const;
        void print_transition_function(
            cpp_container& output,
            const static_transition_data& static_data,
            dynamic_transition_data& dynamic_data,
            const std::vector<state>& local_register)const;
        int get_next_cell(uint current_cell, const rbg_parser::graph& board, const std::vector<precomputed_pattern>& pps)const;// -1 = invalid
        bool is_shift_table_with_multiple_choices(const std::vector<shift_table>& shift_tables)const;
        void print_indices_to_actions_correspondence(
            cpp_container& output,
            const static_transition_data& static_data,
            bool revert_mode,
            bool generate_revert,
            bool last_application)const;
        void print_final_action_effects(cpp_container& output)const;
        std::tuple<bool, std::vector<uint>> build_next_cells_edges(
                uint starting_cell,
                const std::vector<shift_table>& shift_tables,
                const std::vector<precomputed_pattern>& precomputed_patterns,
                const std::vector<std::vector<uint>>& board_structure,
                const std::map<rbg_parser::token, uint>& edges_to_id)const;
        void scan_for_monotonic_moves(std::vector<bool>& visited,
                                      const std::vector<state>& local_register,
                                      const std::vector<shift_table>& shift_tables,
                                      monotonicity_determiner& md)const;
        void scan_first_actions_for_monotonics(const std::vector<state>& local_register,
                                               const std::vector<shift_table>& shift_tables,
                                               monotonicity_determiner& md)const;
};

#endif
