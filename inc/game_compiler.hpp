#ifndef GAME_COMPILER
#define GAME_COMPILER

#include<string>
#include<map>
#include<vector>

#include"cpp_container.hpp"
#include"types.hpp"
#include"automaton.hpp"
#include"shift_table.hpp"
#include"precomputed_pattern.hpp"
#include"cache_checks_container.hpp"

class compiler_options;

namespace rbg_parser{
    class token;
    class parsed_game;
}

class game_compiler{
        cpp_container output;
        const compiler_options& opts;
        std::string name;
        std::map<rbg_parser::token, uint> pieces_to_id;
        std::map<rbg_parser::token, uint> edges_to_id;
        std::map<rbg_parser::token, uint> variables_to_id;
        std::vector<std::vector<uint>> board_structure;
        automaton game_automaton;
        std::vector<automaton> pattern_automata;
        std::vector<shift_table> shift_tables;
        std::vector<precomputed_pattern> precomputed_patterns;
        cache_checks_container ccc;
        std::vector<monotonic_move> monotonic_moves;
        bool uses_pieces_in_arithmetics;
        bool injective_board;
        const rbg_parser::parsed_game& input;
        game_compiler(void)=delete;
        void generate_board_cells_decoder(void);
        void generate_pieces_decoder(void);
        void generate_variables_decoder(void);
        void fill_edges_map(void);
        std::string numbers_to_array(const std::vector<uint>& numbers)const;
        void generate_board_structure(void);
        void generate_appliers_lists(void);
        void generate_actions_applier(void);
        void generate_move_class(void);
        void generate_game_state_class(void);
        void generate_initial_pieces(void);
        void generate_variables_bounds(void);
        void generate_initial_variables(void);
        void generate_state_getters(void);
        void generate_game_parameters(void);
        void build_game_automaton(void);
        void generate_states_iterator(void);
        void generate_iterator_helper_structures(void);
        void generate_main_next_getters(void);
        void generate_pattern_evaluator(uint pattern_index);
        void print_all_shift_tables(void);
        void generate_cache_checks_container(void);
        void generate_revert_info_structure(void);
        void generate_reverter(void);
        void generate_indices_converters(void);
        void generate_monotonic_moves(void);
    public:
        game_compiler(const rbg_parser::parsed_game& input, const compiler_options& opts);
        game_compiler(const game_compiler&)=delete;
        game_compiler(game_compiler&&)=delete;
        game_compiler& operator=(const game_compiler&)=delete;
        game_compiler& operator=(game_compiler&&)=delete;
        ~game_compiler(void)=default;
        const cpp_container& compile(void);
};

#endif
