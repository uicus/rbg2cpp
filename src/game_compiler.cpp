#include"game_compiler.hpp"
#include"parsed_game.hpp"
#include"types.hpp"
#include<algorithm>

game_compiler::game_compiler(const rbg_parser::parsed_game& input, const std::string& output_name):
output(output_name),
name(output_name),
pieces_to_id(),
input(input){
}

void game_compiler::generate_board_cells_decoder(void){
    output.add_header_include("string");
    const auto& g = input.get_board();
    output.add_source_line("static const std::string cells_names["+std::to_string(g.get_size()+1)+"] = {");
    output.add_source_line("\"invalid cell\",");
    for(uint i=0;i<g.get_size();++i)
        output.add_source_line("\""+g.get_vertex(i).to_string()+"\",");
    output.add_source_line("};");
    output.add_source_line("");
    output.add_header_line("std::string board_cell_to_string(int cell_id);");
    output.add_source_line("std::string board_cell_to_string(int cell_id){");
    output.add_source_line("return cells_names[cell_id];");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_pieces_decoder(void){
    output.add_header_include("string");
    const auto& p = input.get_declarations().get_legal_pieces();
    output.add_source_line("static const std::string pieces_names["+std::to_string(p.size())+"] = {");
    uint current_id = 0;
    for(const auto& el: p){
        output.add_source_line("\""+el.to_string()+"\",");
        pieces_to_id.insert(std::make_pair(el, current_id++));
    }
    output.add_source_line("};");
    output.add_source_line("");
    output.add_header_line("std::string piece_to_string(int piece_id);");
    output.add_source_line("std::string piece_to_string(int piece_id){");
    output.add_source_line("return pieces_names[piece_id];");
    output.add_source_line("}");
    output.add_source_line("");
}

const cpp_container& game_compiler::compile(void){
    output.add_header_line("namespace "+name+"{");
    generate_board_cells_decoder();
    generate_pieces_decoder();
    output.add_header_line("class game_state{");
    output.add_header_line("};");
    output.add_header_line("}");
    return output;
}

