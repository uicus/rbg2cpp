#ifndef GAME_COMPILER
#define GAME_COMPILER

#include<string>
#include<map>

#include"cpp_container.hpp"
#include"types.hpp"

namespace rbg_parser{
    class token;
    class parsed_game;
}

class game_compiler{
        cpp_container output;
        std::string name;
        std::map<rbg_parser::token, uint> pieces_to_id;
        std::map<rbg_parser::token, uint> edges_to_id;
        const rbg_parser::parsed_game& input;
        game_compiler(void)=delete;
        void generate_board_cells_decoder(void);
        void generate_pieces_decoder(void);
        void fill_edges_map(void);
        std::string numbers_to_array(const std::vector<int>& numbers)const;
        void generate_board_structure(void);
        void generate_game_state_class(void);
        void generate_initial_pieces(void);
    public:
        game_compiler(const rbg_parser::parsed_game& input, const std::string& output_name);
        game_compiler(const game_compiler&)=default;
        game_compiler(game_compiler&&)=default;
        game_compiler& operator=(const game_compiler&)=default;
        game_compiler& operator=(game_compiler&&)=default;
        ~game_compiler(void)=default;
        const cpp_container& compile(void);
};

#endif
