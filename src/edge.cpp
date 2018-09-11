#include"edge.hpp"
#include"game_move.hpp"
#include"cpp_container.hpp"
#include"actions_compiler.hpp"
#include"declarations.hpp"
#include"state.hpp"
#include"compiler_options.hpp"
#include"graph.hpp"
#include"precomputed_pattern.hpp"
#include"shift_table.hpp"
#include"shift.hpp"
#include"transition_data.hpp"

edge::edge(uint local_register_endpoint_index, const std::vector<label>& label_list):
local_register_endpoint_index(local_register_endpoint_index),
label_list(label_list){
}

void edge::shift(uint shift_value){
    local_register_endpoint_index += shift_value;
}

void edge::inform_abut_state_deletion(uint deleted_index){
    if(local_register_endpoint_index>deleted_index)
        --local_register_endpoint_index;
}

uint edge::get_endpoint(void)const{
    return local_register_endpoint_index;
}

int edge::handle_labels(
    cpp_container& output,
    actions_compiler& ac,
    const static_transition_data& static_data)const{
    for(uint i=0;i<label_list.size();++i){
        const auto& el = label_list[i];
        switch(el.k){
            case action:
                el.a->accept(ac);
                break;
            case positive_pattern:
                ac.finallize();
                output.add_source_line("if(not evaluate"+std::to_string(el.structure_index)+"(current_cell)){");
                ac.insert_reverting_sequence_after_fail();
                output.add_source_line("}");
                break;
            case negative_pattern:
                ac.finallize();
                output.add_source_line("if(evaluate"+std::to_string(el.structure_index)+"(current_cell)){");
                ac.insert_reverting_sequence_after_fail();
                output.add_source_line("}");
                break;
            case s_pattern:
                static_data.precomputed_patterns[el.structure_index].print_inside_transition(output, ac);
                break;
            case s_table:
                if(static_data.shift_tables[el.structure_index].can_be_backtraced()){
                    assert(i==label_list.size()-1);
                    return el.structure_index;
                }
                else{
                    output.add_source_line("current_cell = shift_table"+std::to_string(el.structure_index)+"[current_cell];");
                    ac.notify_about_cell_change();
                }
                break;
            case always_true:
                break;
            case always_false:
                ac.insert_reverting_sequence_after_fail();
                break;
        }
    }
    return -1;
}

void edge::visit_node(
    cpp_container& output,
    const static_transition_data& static_data,
    uint current_state,
    actions_compiler& ac,
    const std::string& cell,
    bool custom_fail_instruction,
    const std::string& fail_instruction)const{
    ac.finallize();
    output.add_source_line("if("+static_data.cache_set_getter+"("+std::to_string(current_state)+", "+cell+"-1)){");
    if(custom_fail_instruction)
        output.add_source_line(fail_instruction);
    else
        ac.insert_reverting_sequence_after_fail();
    output.add_source_line("}");
    output.add_source_line(static_data.cache_setter+"("+std::to_string(current_state)+", "+cell+"-1);");
}

void edge::print_transition_function(
    uint from_state,
    cpp_container& output,
    const static_transition_data& static_data,
    const std::vector<state>& local_register)const{
    output.add_header_line(static_data.return_type+" "+static_data.name_prefix+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(int current_cell);");
    output.add_source_line(static_data.return_type+" next_states_iterator::"+static_data.name_prefix+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(int current_cell){");
    actions_compiler ac(output,static_data);
    int last_state_to_check = -1;
    int encountered_multi_shift_table = handle_labels(output,ac,static_data);
    uint current_state = local_register_endpoint_index;
    while(not ac.is_ready_to_report() and local_register[current_state].is_no_choicer() and encountered_multi_shift_table == -1){
        if(local_register[current_state].can_be_checked_for_visit()){
            if(local_register[current_state].get_only_exit().label_list.empty())
                last_state_to_check = current_state;
            else{
                visit_node(output,static_data,current_state, ac);
                last_state_to_check = -1;
            }
        }
        else if(not local_register[current_state].get_only_exit().label_list.empty() and last_state_to_check >= 0){
            visit_node(output,static_data,last_state_to_check,ac);
            last_state_to_check = -1;
        }
        encountered_multi_shift_table = local_register[current_state].get_only_exit().handle_labels(output,ac,static_data);
        current_state = local_register[current_state].get_only_exit().local_register_endpoint_index;
    }
    if(encountered_multi_shift_table >= 0){
        ac.notify_about_modifier();
        output.add_source_line("for(const auto el: shift_table"+std::to_string(encountered_multi_shift_table)+"[current_cell]){");
        visit_node(output,static_data,current_state,ac,"el",true,"continue;");
        local_register[current_state].print_recursive_calls(current_state,output,static_data,ac,"el");
        output.add_source_line("}");
        ac.insert_reverting_sequence_after_fail();
        output.add_source_line("}");
        output.add_source_line("");
        return;
    }
    if(local_register[current_state].can_be_checked_for_visit())
        last_state_to_check = current_state;
    if(last_state_to_check >= 0)
        visit_node(output,static_data,last_state_to_check,ac);
    ac.finallize();
    if(ac.is_ready_to_report()){
        output.add_source_line("moves.emplace_back(board_list,variables_list,"+std::to_string(ac.get_next_player())+",current_cell,"+std::to_string(current_state)+");");
        ac.insert_reverting_sequence_after_success();
    }
    else{
        local_register[current_state].print_recursive_calls(current_state,output,static_data,ac);
        ac.insert_reverting_sequence_after_fail();
    }
    output.add_source_line("}");
    output.add_source_line("");
}

void edge::print_transition_function_inside_pattern(
    uint from_state,
    cpp_container& output,
    const static_transition_data& static_data,
    const std::vector<state>& local_register)const{
    output.add_header_line(static_data.return_type+" "+static_data.name_prefix+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(int current_cell);");
    output.add_source_line(static_data.return_type+" next_states_iterator::"+static_data.name_prefix+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(int current_cell){");
    actions_compiler ac(output,static_data);
    int last_state_to_check = -1;
    int encountered_multi_shift_table = handle_labels(output,ac,static_data);
    uint current_state = local_register_endpoint_index;
    while(local_register[current_state].is_no_choicer() and encountered_multi_shift_table == -1){
        if(local_register[current_state].can_be_checked_for_visit()){
            if(local_register[current_state].get_only_exit().label_list.empty())
                last_state_to_check = current_state;
            else{
                visit_node(output,static_data,current_state,ac);
                last_state_to_check = -1;
            }
        }
        else if(not local_register[current_state].get_only_exit().label_list.empty() and last_state_to_check >= 0){
            visit_node(output,static_data,last_state_to_check,ac);
            last_state_to_check = -1;
        }
        encountered_multi_shift_table = local_register[current_state].get_only_exit().handle_labels(output,ac,static_data);
        current_state = local_register[current_state].get_only_exit().local_register_endpoint_index;
    }
    if(encountered_multi_shift_table >= 0){
        ac.notify_about_modifier();
        output.add_source_line("for(const auto el: shift_table"+std::to_string(encountered_multi_shift_table)+"[current_cell]){");
        visit_node(output,static_data,current_state,ac,"el",true,"continue;");
        local_register[current_state].print_recursive_calls(current_state,output,static_data,ac,"el");
        output.add_source_line("}");
        ac.insert_reverting_sequence_after_fail();
        output.add_source_line("}");
        output.add_source_line("");
        return;
    }
    if(local_register[current_state].can_be_checked_for_visit())
        last_state_to_check = current_state;
    if(last_state_to_check >= 0)
        visit_node(output,static_data,last_state_to_check,ac);
    ac.finallize();
    assert(not ac.is_ready_to_report());
    if(local_register[current_state].is_dead_end())
        ac.insert_reverting_sequence_after_success();
    else{
        local_register[current_state].print_recursive_calls(current_state, output, static_data, ac);
        ac.insert_reverting_sequence_after_fail();
    }
    output.add_source_line("}");
    output.add_source_line("");
}

int edge::get_next_cell(uint current_cell, const rbg_parser::graph& board, const std::vector<precomputed_pattern>& pps)const{
    for(const auto& el: label_list)
        switch(el.k){
            case action:
                {
                    const auto& outgoing_edges = board.get_outgoing_edges(current_cell);
                    const rbg_parser::shift* s = dynamic_cast<const rbg_parser::shift*>(el.a);
                    auto it = outgoing_edges.find(s->get_content());
                    if(it == outgoing_edges.end())
                        return -1;
                    current_cell = it->second;
                }
                break;
            case positive_pattern:
            case negative_pattern:
                assert(false);
                break;
            case s_pattern:
                if(not pps[el.structure_index].evaluates_to_true(current_cell))
                    return -1;
                break;
            case s_table:
                assert(false); // no recursive shift tables
            case always_true:
                break;
            case always_false:
                return -1;
                break;
        }
    return current_cell;
}

bool edge::is_shift_table_with_multiple_choices(const std::vector<shift_table>& shift_tables)const{
    return label_list.size() == 1 and label_list[0].k == s_table and shift_tables[label_list[0].structure_index].can_be_backtraced();
}
