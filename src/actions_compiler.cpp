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
#include"transition_data.hpp"
#include"compiler_options.hpp"

actions_compiler::actions_compiler(
    cpp_container& output,
    const static_transition_data& static_data,
    dynamic_transition_data& dynamic_data):
    output(output),
    static_data(static_data),
    dynamic_data(dynamic_data){
}

void actions_compiler::dispatch(const rbg_parser::shift& m){
    output.add_source_line("cell = cell_neighbors[cell]["+std::to_string(static_data.edges_to_id.at(m.get_content()))+"];");
    dynamic_data.queue_cell_check();
}

void actions_compiler::dispatch(const rbg_parser::noop&){
    dynamic_data.visit_custom_split_point(-1);
}

void actions_compiler::dispatch(const rbg_parser::off& m){
    dynamic_data.visit_custom_split_point(m.index_in_expression());// Added
    
    dynamic_data.handle_cell_check(output);
    dynamic_data.save_board_change_for_later_revert(output,static_data.pieces_to_id.at(m.get_piece()));
    dynamic_data.push_any_change_on_modifiers_list(output, std::to_string(m.index_in_expression()), "cell");
    if(static_data.uses_pieces_in_arithmetics){
        output.add_source_line("--pieces_count[pieces[cell]];");
        output.add_source_line("++pieces_count["+std::to_string(static_data.pieces_to_id.at(m.get_piece()))+"];");
    }
    output.add_source_line("pieces[cell] = "+std::to_string(static_data.pieces_to_id.at(m.get_piece()))+";");
}

void actions_compiler::dispatch(const rbg_parser::ons& m){
    if(m.get_legal_ons().size() == 0)
        dynamic_data.insert_reverting_sequence_after_fail(output);
    else if(m.get_legal_ons().size() < static_data.pieces_to_id.size()){
        dynamic_data.clear_queue_checks();
        output.add_source_line("switch(pieces[cell]){");
        if(m.get_legal_ons().size() < static_data.pieces_to_id.size()/2+1){
            for(const auto& el: m.get_legal_ons())
                output.add_source_line("case "+std::to_string(static_data.pieces_to_id.at(el))+":");
            output.add_source_line("break;");
            output.add_source_line("default:");
            dynamic_data.insert_reverting_sequence_after_fail(output);
        }
        else{
            for(const auto& el: static_data.pieces_to_id)
                if(not m.get_legal_ons().count(el.first))
                    output.add_source_line("case "+std::to_string(el.second)+":");
            output.add_source_line("case -1:");
            dynamic_data.insert_reverting_sequence_after_fail(output);
            output.add_source_line("default:");
            output.add_source_line("break;");
        }
        output.add_source_line("}");
    }
}

void actions_compiler::print_variable_assignment(uint variable_id, const std::string& rvalue, const std::string& action_index){
    dynamic_data.save_variable_change_for_later_revert(output, variable_id);
    dynamic_data.push_any_change_on_modifiers_list(output, action_index, "cell");
    output.add_source_line("variables["+std::to_string(variable_id)+"] = "+rvalue+";");
}

void actions_compiler::dispatch(const rbg_parser::assignment& m){
    dynamic_data.visit_custom_split_point(m.index_in_expression());// Added

    const auto& left_side = m.get_left_side();
    uint bound = 0;
    if(static_data.decl.get_legal_variables().count(left_side))
        bound = static_data.decl.get_variable_bound(left_side);
    else
        bound = static_data.decl.get_player_bound(left_side);
    arithmetics_printer right_side_printer(static_data.pieces_to_id, static_data.variables_to_id, "");
    m.get_right_side()->accept(right_side_printer);
    output.add_source_line("// Potential assignment value check");// Added
    if(right_side_printer.can_be_precomputed()){
        if(right_side_printer.precomputed_value() < 0 or right_side_printer.precomputed_value() > int(bound))
            dynamic_data.insert_reverting_sequence_after_fail(output);
        else
            print_variable_assignment(static_data.variables_to_id.at(left_side),std::to_string(right_side_printer.precomputed_value()),std::to_string(m.index_in_expression()));
    }
    else{
        std::string final_result = right_side_printer.get_final_result();
        output.add_source_line("if("+final_result+" > bounds["+std::to_string(static_data.variables_to_id.at(left_side))+"] or "+final_result+" <0){");
        dynamic_data.insert_reverting_sequence_after_fail(output);
        output.add_source_line("}");
        print_variable_assignment(static_data.variables_to_id.at(left_side),final_result, std::to_string(m.index_in_expression()));
    }
}

void actions_compiler::dispatch(const rbg_parser::player_switch& m){
    dynamic_data.push_any_change_on_modifiers_list(output, std::to_string(m.index_in_expression()), "cell");
    dynamic_data.set_next_player(static_data.variables_to_id.at(m.get_player())+1);
}

void actions_compiler::dispatch(const rbg_parser::keeper_switch& m){
    dynamic_data.push_any_change_on_modifiers_list(output, std::to_string(m.index_in_expression()), "cell");
    dynamic_data.set_next_player(0);
}

void actions_compiler::dispatch(const rbg_parser::arithmetic_comparison& m){
    arithmetics_printer left_side_printer(static_data.pieces_to_id, static_data.variables_to_id, "");
    arithmetics_printer right_side_printer(static_data.pieces_to_id, static_data.variables_to_id, "");
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
        if(not can_pass_through)
            dynamic_data.insert_reverting_sequence_after_fail(output);
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
        dynamic_data.insert_reverting_sequence_after_fail(output);
        output.add_source_line("}");
    }
}
