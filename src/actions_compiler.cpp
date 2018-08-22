#include"actions_compiler.hpp"
#include"declarations.hpp"
#include"cpp_container.hpp"
#include"arithmetics_printer.hpp"
#include"game_move.hpp"
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
    const rbg_parser::declarations& decl,
    const std::string& reverting_function,
    const std::string& cache_pusher,
    bool should_build_move):
    output(output),
    pieces_to_id(pieces_to_id),
    edges_to_id(edges_to_id),
    variables_to_id(variables_to_id),
    decl(decl),
    reverting_function(reverting_function),
    cache_pusher(cache_pusher),
    should_check_cell_correctness(false),
    has_modifier(false),
    is_finisher(false),
    should_build_move(should_build_move){
}

void actions_compiler::dispatch(const rbg_parser::shift& m){
    output.add_source_line("state_to_change.current_cell = cell_neighbors[state_to_change.current_cell]["+std::to_string(edges_to_id.at(m.get_content()))+"];");
    should_check_cell_correctness = true;
}

void actions_compiler::dispatch(const rbg_parser::off& m){
    check_cell_correctness();
    output.add_source_line("board_change_points.emplace_back(state_to_change.current_cell, state_to_change.pieces[state_to_change.current_cell]);");
    output.add_source_line("--state_to_change.pieces_count[state_to_change.pieces[state_to_change.current_cell]];");
    output.add_source_line("++state_to_change.pieces_count["+std::to_string(pieces_to_id.at(m.get_piece()))+"];");
    output.add_source_line("state_to_change.pieces[state_to_change.current_cell] = "+std::to_string(pieces_to_id.at(m.get_piece()))+";");
    if(should_build_move)
        output.add_source_line("board_list = std::make_shared<board_appliers>(state_to_change.current_cell,"+std::to_string(pieces_to_id.at(m.get_piece()))+",board_list);");
    has_modifier = true;
}

void actions_compiler::dispatch(const rbg_parser::ons& m){
    if(m.get_legal_ons().size() == 0){
        output.add_source_line(reverting_function);
        output.add_source_line("return;");
    }
    else if(m.get_legal_ons().size() < pieces_to_id.size()){
        check_cell_correctness();
        output.add_source_line("switch(state_to_change.pieces[state_to_change.current_cell]){");
        if(m.get_legal_ons().size() < pieces_to_id.size()/2+1){
            for(const auto& el: m.get_legal_ons())
                output.add_source_line("case "+std::to_string(pieces_to_id.at(el))+":");
            output.add_source_line("break;");
            output.add_source_line("default:");
            output.add_source_line(reverting_function);
            output.add_source_line("return;");
        }
        else{
            for(const auto& el: pieces_to_id)
                if(not m.get_legal_ons().count(el.first))
                    output.add_source_line("case "+std::to_string(el.second)+":");
            output.add_source_line(reverting_function);
            output.add_source_line("return;");
            output.add_source_line("default:");
            output.add_source_line("break;");
        }
        output.add_source_line("}");
    }
}

void actions_compiler::dispatch(const rbg_parser::assignment& m){
    const auto& left_side = m.get_left_side();
    uint bound = 0;
    if(decl.get_legal_variables().count(left_side))
        bound = decl.get_variable_bound(left_side);
    else
        bound = decl.get_player_bound(left_side);
    arithmetics_printer right_side_printer(pieces_to_id, variables_to_id);
    m.get_right_side()->accept(right_side_printer);
    if(right_side_printer.can_be_precomputed()){
        if(right_side_printer.precomputed_value() < 0 or right_side_printer.precomputed_value() > int(bound)){
            output.add_source_line(reverting_function);
            output.add_source_line("return;");
        }
        else{
            output.add_source_line("variables_change_points.emplace_back("+std::to_string(variables_to_id.at(left_side))+", state_to_change.variables["+std::to_string(variables_to_id.at(left_side))+"]);");
            output.add_source_line("state_to_change.variables["+std::to_string(variables_to_id.at(left_side))+"] = "+std::to_string(right_side_printer.precomputed_value())+";");
            if(should_build_move)
                output.add_source_line("variables_list = std::make_shared<variables_appliers>("+std::to_string(variables_to_id.at(left_side))+","+std::to_string(right_side_printer.precomputed_value())+",variables_list);");
        }
    }
    else{
            output.add_source_line("variables_change_points.emplace_back("+std::to_string(variables_to_id.at(left_side))+", state_to_change.variables["+std::to_string(variables_to_id.at(left_side))+"]);");
            std::string final_result = right_side_printer.get_final_result();
            output.add_source_line("state_to_change.variables["+std::to_string(variables_to_id.at(left_side))+"] = "+final_result+";");
            if(should_build_move)
                output.add_source_line("variables_list = std::make_shared<variables_appliers>("+std::to_string(variables_to_id.at(left_side))+","+final_result+",variables_list);");
            output.add_source_line("if(state_to_change.variables["+std::to_string(variables_to_id.at(left_side))+"] > bounds["+std::to_string(variables_to_id.at(left_side))+"]){");
            output.add_source_line(reverting_function);
            output.add_source_line("return;");
            output.add_source_line("}");
    }
    has_modifier = true;
}

void actions_compiler::dispatch(const rbg_parser::player_switch& m){
    output.add_source_line("state_to_change.current_player = "+std::to_string(variables_to_id.at(m.get_player())+1)+";");
    is_finisher = true;
}

void actions_compiler::dispatch(const rbg_parser::keeper_switch&){
    output.add_source_line("state_to_change.current_player = 0;");
    is_finisher = true;
}

void actions_compiler::dispatch(const rbg_parser::arithmetic_comparison& m){
    arithmetics_printer left_side_printer(pieces_to_id, variables_to_id);
    arithmetics_printer right_side_printer(pieces_to_id, variables_to_id);
    m.get_left_side()->accept(left_side_printer);
    m.get_right_side()->accept(right_side_printer);
    if(left_side_printer.can_be_precomputed() and right_side_printer.can_be_precomputed()){
        bool can_pass_through = false;
        switch(m.get_kind_of_comparison()){
            case rbg_parser::eq:
                can_pass_through = (left_side_printer.precomputed_value() == right_side_printer.precomputed_value());
                break;
            case rbg_parser::neq:
                can_pass_through = (left_side_printer.precomputed_value() != right_side_printer.precomputed_value());
                break;
            case rbg_parser::le:
                can_pass_through = (left_side_printer.precomputed_value() < right_side_printer.precomputed_value());
                break;
            case rbg_parser::leq:
                can_pass_through = (left_side_printer.precomputed_value() <= right_side_printer.precomputed_value());
                break;
            case rbg_parser::ge:
                can_pass_through = (left_side_printer.precomputed_value() > right_side_printer.precomputed_value());
                break;
            case rbg_parser::geq:
                can_pass_through = (left_side_printer.precomputed_value() >= right_side_printer.precomputed_value());
                break;
            default:
                break;
        }
        if(not can_pass_through){
            output.add_source_line(reverting_function);
            output.add_source_line("return;");
        }
    }
    else{
        std::string operation_character = "";
        switch(m.get_kind_of_comparison()){
            case rbg_parser::eq:
                operation_character = "!=";
                break;
            case rbg_parser::neq:
                operation_character = "==";
                break;
            case rbg_parser::le:
                operation_character = ">=";
                break;
            case rbg_parser::leq:
                operation_character = ">";
                break;
            case rbg_parser::ge:
                operation_character = "<=";
                break;
            case rbg_parser::geq:
                operation_character = "<";
                break;
            default:
                break;
        }
        output.add_source_line("if("+left_side_printer.get_final_result()+" "+operation_character+" "+right_side_printer.get_final_result()+"){");
        output.add_source_line(reverting_function);
        output.add_source_line("return;");
        output.add_source_line("}");
    }
}

void actions_compiler::finallize(void){
    check_cell_correctness();
    notify_about_modifier();
}

void actions_compiler::check_cell_correctness(void){
    if(should_check_cell_correctness){
        output.add_source_line("if(state_to_change.current_cell == 0){");
        output.add_source_line(reverting_function);
        output.add_source_line("return;");
        output.add_source_line("}");
    }
    should_check_cell_correctness = false;
}

void actions_compiler::notify_about_modifier(void){
    if(has_modifier)
        output.add_source_line("cache."+cache_pusher+";");
    has_modifier = false;
}

bool actions_compiler::is_ready_to_report(void)const{
    return is_finisher;
}
