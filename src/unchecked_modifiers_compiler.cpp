#include"unchecked_modifiers_compiler.hpp"
#include"cpp_container.hpp"
#include"offs.hpp"
#include"assignments.hpp"
#include"switch.hpp"
#include"transition_data.hpp"
#include"arithmetics_printer.hpp"
#include"compiler_options.hpp"

unchecked_modifiers_compiler::unchecked_modifiers_compiler(cpp_container& output,
                                                           const static_transition_data& static_data,
                                                           uint next_state_index,
                                                           bool revert_mode,
                                                           bool generate_revert,
                                                           bool last_application)
  : output(output),
    static_data(static_data),
    next_state_index(next_state_index),
    revert_mode(revert_mode),
    generate_revert(generate_revert),
    last_application(last_application) {}

void unchecked_modifiers_compiler::dispatch(const rbg_parser::off& m){
    output.add_source_line("case "+std::to_string(m.index_in_expression())+":");
    if (revert_mode) {
        if(static_data.uses_pieces_in_arithmetics){
            output.add_source_line("--pieces_count[pieces[current_cell]];");
            output.add_source_line("++pieces_count[modrev.previous_value];");
        }
        output.add_source_line("pieces[current_cell] = modrev.previous_value;");
        output.add_source_line("current_cell = modrev.previous_cell;");
    } else {
        if(generate_revert) {
            output.add_source_line("modrev.previous_value = pieces[action.cell];");
        }
        if(static_data.uses_pieces_in_arithmetics){
            output.add_source_line("--pieces_count[pieces[action.cell]];");
            output.add_source_line("++pieces_count["+std::to_string(static_data.pieces_to_id.at(m.get_piece()))+"];");
        }
        output.add_source_line("pieces[action.cell] = "+std::to_string(static_data.pieces_to_id.at(m.get_piece()))+";");
        if (generate_revert || last_application) output.add_source_line("current_cell = action.cell;");
        if (last_application) output.add_source_line("current_state = "+std::to_string(next_state_index)+";");
    }
    output.add_source_line("break;");
}

void unchecked_modifiers_compiler::dispatch(const rbg_parser::assignment& m){
    output.add_source_line("case "+std::to_string(m.index_in_expression())+":");
    const auto& left_side = m.get_left_side();
    const std::string left_variable_name = std::to_string(static_data.variables_to_id.at(left_side));
    if (revert_mode) {
        output.add_source_line("variables["+left_variable_name+"] = modrev.previous_value;");
        output.add_source_line("current_cell = modrev.previous_cell;");
    } else {
        if(generate_revert) {
            output.add_source_line("modrev.previous_value = variables["+left_variable_name+"];");
        }
        arithmetics_printer right_side_printer(static_data.pieces_to_id, static_data.variables_to_id,"");
        m.get_right_side()->accept(right_side_printer);
        if(right_side_printer.can_be_precomputed()){
            output.add_source_line("variables["+left_variable_name+"] = "+std::to_string(right_side_printer.precomputed_value())+";");
        }
        else{
            std::string final_result = right_side_printer.get_final_result();
            output.add_source_line("variables["+left_variable_name+"] = "+final_result+";");
        }
        if (generate_revert || last_application) output.add_source_line("current_cell = action.cell;");
        if (last_application) output.add_source_line("current_state = "+std::to_string(next_state_index)+";");
    }
        output.add_source_line("break;");
}

void unchecked_modifiers_compiler::dispatch(const rbg_parser::player_switch& m){
    output.add_source_line("case "+std::to_string(m.index_in_expression())+":");
    if (revert_mode) {
        output.add_source_line("current_player = modrev.previous_value;");
        output.add_source_line("current_cell = modrev.previous_cell;");
    } else {
        if (generate_revert) {
            output.add_source_line("modrev.previous_value = current_player;");
        }
        output.add_source_line("current_player = "+std::to_string(static_data.variables_to_id.at(m.get_player())+1)+";");
        output.add_source_line("current_cell = action.cell;");
        output.add_source_line("current_state = "+std::to_string(next_state_index)+";");
    }
    output.add_source_line("break;");
}

void unchecked_modifiers_compiler::dispatch(const rbg_parser::keeper_switch& m){
    output.add_source_line("case "+std::to_string(m.index_in_expression())+":");
    if (revert_mode) {
        output.add_source_line("current_player = modrev.previous_value;");
        output.add_source_line("current_cell = modrev.previous_cell;");
    } else {
        if (generate_revert) {
            output.add_source_line("modrev.previous_value = current_player;");
        }
        output.add_source_line("current_player = 0;");
        output.add_source_line("current_cell = action.cell;");
        output.add_source_line("current_state = "+std::to_string(next_state_index)+";");
    }
    output.add_source_line("break;");
}
