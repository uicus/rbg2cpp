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
#include"unchecked_modifiers_compiler.hpp"

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

void edge::handle_labels(
    cpp_container& output,
    const static_transition_data& static_data,
    dynamic_transition_data& dynamic_data)const{
    actions_compiler ac(output,static_data,dynamic_data);
    for(uint i=0;i<label_list.size();++i){
        const auto& el = label_list[i];
        switch(el.k){
            case action:
                el.a->accept(ac);
                break;
            case positive_pattern:
                dynamic_data.finallize(output);
                output.add_source_line("if(not evaluate"+std::to_string(el.structure_index)+"(cell, cache)){");
                dynamic_data.insert_reverting_sequence_after_fail(output);
                output.add_source_line("}");
                break;
            case negative_pattern:
                dynamic_data.finallize(output);
                output.add_source_line("if(evaluate"+std::to_string(el.structure_index)+"(cell, cache)){");
                dynamic_data.insert_reverting_sequence_after_fail(output);
                output.add_source_line("}");
                break;
            case s_pattern:
                static_data.precomputed_patterns[el.structure_index].print_inside_transition(output, dynamic_data);
                break;
            case s_table:
                if(static_data.shift_tables[el.structure_index].can_be_backtraced()){
                    assert(i==label_list.size()-1);
                    dynamic_data.queue_branching_shift_table(el.structure_index);
                }
                else{
                    output.add_source_line("cell = shift_table"+std::to_string(el.structure_index)+"[cell];");
                    dynamic_data.queue_cell_check();
                }
                break;
            case always_true:
                break;
            case always_false:
                dynamic_data.insert_reverting_sequence_after_fail(output);
                break;
        }
    }
}

void edge::print_transition_function(
    cpp_container& output,
    const static_transition_data& static_data,
    dynamic_transition_data& dynamic_data,
    const std::vector<state>& local_register)const{
    output.add_header_line(static_data.return_type+" "+static_data.name_prefix+dynamic_data.get_start_state()+"_"+std::to_string(local_register_endpoint_index)+"(int cell, resettable_bitarray_stack& cache"+(static_data.kind == all_getter ? ", move_representation& mr, std::vector<move>& moves" : "")+");");
    std::string arguments = "[[maybe_unused]] int cell, [[maybe_unused]] resettable_bitarray_stack& cache";
    if(static_data.kind == all_getter)
        arguments += ", [[maybe_unused]] move_representation& mr, [[maybe_unused]] std::vector<move>& moves";
    output.add_source_line(static_data.return_type+" game_state::"+static_data.name_prefix+dynamic_data.get_start_state()+"_"+std::to_string(local_register_endpoint_index)+"("+arguments+"){");
    actions_compiler ac(output,static_data,dynamic_data);
    handle_labels(output,static_data,dynamic_data);
    uint current_state = local_register_endpoint_index;
    while(dynamic_data.can_handle_further_labels() and local_register[current_state].is_no_choicer()){
        if(local_register[current_state].can_be_checked_for_visit()){
            dynamic_data.queue_state_to_check_visited(current_state);
            if(not local_register[current_state].get_only_exit().label_list.empty())
                dynamic_data.visit_node(output);
        }
        else if(not local_register[current_state].get_only_exit().label_list.empty() and dynamic_data.should_check_for_visited())
            dynamic_data.visit_node(output);
        local_register[current_state].get_only_exit().handle_labels(output,static_data,dynamic_data);
        current_state = local_register[current_state].get_only_exit().local_register_endpoint_index;
    }
    if(dynamic_data.should_handle_branching_shift_table())
        dynamic_data.handle_branching_shift_table(output,local_register[current_state],current_state);
    else
        dynamic_data.handle_standard_transition_end(output,local_register[current_state],current_state);
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

void edge::print_indices_to_actions_correspondence(
    cpp_container& output,
    const static_transition_data& static_data)const{
    unchecked_modifiers_compiler umc(output,static_data,local_register_endpoint_index);
    for(const auto& el: label_list)
        switch(el.k){
            case action:
                el.a->accept(umc);
                break;
            case positive_pattern:
            case negative_pattern:
            case s_pattern:
            case s_table:
            case always_true:
            case always_false:
                break;
        }
}
