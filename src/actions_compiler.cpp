#include"actions_compiler.hpp"
#include"declarations.hpp"
#include"cpp_container.hpp"
#include"game_move.hpp"
#include"actions_block.hpp"
#include"shift.hpp"
#include"ons.hpp"
#include"offs.hpp"
#include"assignments.hpp"
#include"switch.hpp"
#include"arithmetic_comparison.hpp"

actions_compiler::actions_compiler(
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl):
    output(output),
    pieces_to_id(pieces_to_id),
    edges_to_id(edges_to_id),
    variables_to_id(variables_to_id),
    decl(decl){
}

void actions_compiler::dispatch(const rbg_parser::shift& m){
    output.add_source_line("state_to_change.current_cell = cell_neighbors["+std::to_string(edges_to_id.at(m.get_content()))+"];");
}

void actions_compiler::dispatch(const rbg_parser::off& m){
    // check for cell correctness if previously encountered shift
    output.add_source_line("board_change_points.emplace_back(state_to_change.current_cell, state_to_change.pieces[state_to_change.current_cell]);");
    output.add_source_line("--state_to_change.pieces_count[state_to_change.pieces[state_to_change.current_cell]];");
    output.add_source_line("++state_to_change.pieces_count["+std::to_string(pieces_to_id.at(m.get_piece()))+"];");
    output.add_source_line("state_to_change.pieces[state_to_change.current_cell] = "+std::to_string(pieces_to_id.at(m.get_piece()))+";");
}

void actions_compiler::dispatch(const rbg_parser::ons& m){
    output.add_source_line("switch(state_to_change.pieces[state_to_change.current_cell]){");
    if(m.get_legal_ons().size() < pieces_to_id.size()/2){
        for(const auto& el: m.get_legal_ons())
            output.add_source_line("case "+std::to_string(pieces_to_id.at(el))+":");
        output.add_source_line("// revert here");
        output.add_source_line("return;");
        output.add_source_line("default:");
        output.add_source_line("break;");
    }
    else{
        for(const auto& el: decl.get_legal_pieces())
            if(not m.get_legal_ons().count(el))
                output.add_source_line("case "+std::to_string(pieces_to_id.at(el))+":");
        output.add_source_line("break;");
        output.add_source_line("default:");
        output.add_source_line("// revert here");
        output.add_source_line("return;");
    }
    output.add_source_line("}");
}
