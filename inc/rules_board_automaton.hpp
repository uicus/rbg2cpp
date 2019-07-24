#ifndef RULES_BOARD_AUTOMATON
#define RULES_BOARD_AUTOMATON

#include"types.hpp"

#include<vector>
#include<map>
#include"resettable_bitarray.hpp"

class edge;
class shift_table;
class precomputed_pattern;
class edge;

namespace rbg_parser{
    class token;
}

class rules_board_automaton{
        struct edge_info{
            uint target_state;
            uint target_cell;
            bool crosses_modifier;
        };
        struct state_info{
            std::vector<edge_info> edges = {};
        };
        std::vector<bool> potential_starter;
        std::vector<bool> check_needed;
        resettable_bitarray visited;
        std::vector<std::vector<state_info>> states;
        void run_dfs_from(uint state, uint cell);
    public:
        rules_board_automaton(void)=delete;
        rules_board_automaton(const rules_board_automaton&)=delete;
        rules_board_automaton& operator=(const rules_board_automaton&)=delete;
        rules_board_automaton(rules_board_automaton&&)=default;
        rules_board_automaton& operator=(rules_board_automaton&&)=default;
        ~rules_board_automaton(void)=default;
        rules_board_automaton(uint num_of_states, uint num_of_cells);
        std::vector<bool> get_cache_checks_need(void);
        void report_edge_from_original_automaton(
            const edge& e,
            uint starting_state,
            const std::vector<shift_table>& shift_tables,
            const std::vector<precomputed_pattern>& precomputed_patterns,
            const std::vector<std::vector<uint>>& board_structure,
            const std::map<rbg_parser::token, uint>& edges_to_id);
};

#endif
