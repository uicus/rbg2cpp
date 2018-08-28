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
    actions_compiler& ac,
    const std::vector<shift_table>& shift_tables,
    const std::vector<precomputed_pattern>& precomputed_patterns,
    const std::string& decision_stack_name,
    std::string& human_readable_labels)const{
    for(const auto& el: label_list)
        switch(el.k){
            case action:
                human_readable_labels += el.a->to_rbg();
                el.a->accept(ac);
                break;
            case positive_pattern:
                ac.finallize();
                output.add_source_line("evaluate"+std::to_string(el.structure_index)+"();");
                output.add_source_line("if(not success_to_report"+std::to_string(el.structure_index)+"){");
                ac.insert_reverting_sequence(output);
                output.add_source_line("return;");
                output.add_source_line("}");
                break;
            case negative_pattern:
                ac.finallize();
                output.add_source_line("evaluate"+std::to_string(el.structure_index)+"();");
                output.add_source_line("if(success_to_report"+std::to_string(el.structure_index)+"){");
                ac.insert_reverting_sequence(output);
                output.add_source_line("return;");
                output.add_source_line("}");
                break;
            case s_pattern:
                precomputed_patterns[el.structure_index].print_inside_transition(output, ac);
                break;
            case s_table:
                if(shift_tables[el.structure_index].can_be_backtraced()){
                    output.add_source_line("if("+decision_stack_name+".back().current_shift_table_branch == shift_table"+std::to_string(el.structure_index)+"[state_to_change.current_cell].size()){");
                    output.add_source_line(decision_stack_name+".back().current_shift_table_branch = 0;");
                    ac.insert_reverting_sequence(output);
                    output.add_source_line("return;");
                    output.add_source_line("}");
                    output.add_source_line("else{");
                    output.add_source_line("--"+decision_stack_name+".back().current_branch;");
                    output.add_source_line("}");
                    output.add_source_line("state_to_change.current_cell = shift_table"+std::to_string(el.structure_index)+"[state_to_change.current_cell]["+decision_stack_name+".back().current_shift_table_branch++];");
                }
                else{
                    output.add_source_line("state_to_change.current_cell = shift_table"+std::to_string(el.structure_index)+"[state_to_change.current_cell];");
                    ac.notify_about_cell_change();
                }
                break;
            case always_true:
                break;
            case always_false:
                ac.insert_reverting_sequence(output);
                output.add_source_line("return;");
                break;
        }
}

void edge::visit_node(cpp_container& output, uint current_state, actions_compiler& ac)const{
    ac.finallize();
    output.add_source_line("if(cache.is_set("+std::to_string(current_state)+", state_to_change.current_cell-1)){");
    ac.insert_reverting_sequence(output);
    output.add_source_line("return;");
    output.add_source_line("}");
    output.add_source_line("cache.set("+std::to_string(current_state)+", state_to_change.current_cell-1);");
}

void edge::visit_node_in_pattern(cpp_container& output, uint current_state, uint pattern_index, actions_compiler& ac)const{
    ac.finallize();
    output.add_source_line("if(cache.pattern_is_set"+std::to_string(pattern_index)+"("+std::to_string(current_state)+", state_to_change.current_cell-1)){");
    ac.insert_reverting_sequence(output);
    output.add_source_line("return;");
    output.add_source_line("}");
    output.add_source_line("cache.pattern_set"+std::to_string(pattern_index)+"("+std::to_string(current_state)+", state_to_change.current_cell-1);");
}

void edge::print_transition_function(
    uint from_state,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::vector<state>& local_register,
    const std::vector<shift_table>& shift_tables,
    const std::vector<precomputed_pattern>& precomputed_patterns,
    const compiler_options& opts)const{
    output.add_header_line("void transition_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(void);");
    output.add_source_line("void next_states_iterator::transition_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(void){");
    if(local_register[local_register_endpoint_index].is_dead_end()){
        output.add_source_line("revert_to_last_choice();");
    }
    else{
        std::string pusher = "push()";
        std::string level_getter = "get_level()";
        std::string level_reverter = "revert_to_level";
        std::string decision_stack = "decision_points";
        actions_compiler ac(output,pieces_to_id,edges_to_id,variables_to_id,decl,pusher,level_getter,level_reverter,false);
        int last_state_to_check = -1;
        std::string human_readable_labels;
        handle_labels(output,ac,shift_tables,precomputed_patterns,decision_stack,human_readable_labels);
        uint current_state = local_register_endpoint_index;
        while(not ac.is_ready_to_report() and local_register[current_state].is_no_choicer(shift_tables)){
            if(local_register[current_state].can_be_checked_for_visit()){
                if(local_register[current_state].get_only_exit().label_list.empty())
                    last_state_to_check = current_state;
                else{
                    visit_node(output, current_state,ac);
                    last_state_to_check = -1;
                }
            }
            else if(not local_register[current_state].get_only_exit().label_list.empty() and last_state_to_check >= 0){
                visit_node(output,last_state_to_check,ac);
                last_state_to_check = -1;
            }
            local_register[current_state].get_only_exit().handle_labels(output,ac,shift_tables,precomputed_patterns,decision_stack,human_readable_labels);
            current_state = local_register[current_state].get_only_exit().local_register_endpoint_index;
        }
        if(local_register[current_state].can_be_checked_for_visit())
            last_state_to_check = current_state;
        if(last_state_to_check >= 0)
            visit_node(output, last_state_to_check,ac);
        ac.finallize();
        if(ac.is_ready_to_report()){
            output.add_source_line("ready_to_report = true;");
            output.add_source_line("state_to_change.current_state = "+std::to_string(current_state)+";");
        }
        else{
            output.add_source_line("decision_points.emplace_back(0,"+(opts.enabled_shift_tables()?"0,":std::string())+std::to_string(current_state)+",state_to_change.current_cell,cache.get_level(),board_change_points.size(),variables_change_points.size());");
        }
        output.add_source_line("// generated from: "+human_readable_labels);
    }
    output.add_source_line("}");
    output.add_source_line("");
}

void edge::print_transition_function_inside_pattern(
    uint from_state,
    uint pattern_index,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::vector<state>& local_register,
    const std::vector<shift_table>& shift_tables,
    const std::vector<precomputed_pattern>& precomputed_patterns,
    const compiler_options& opts)const{
    output.add_header_line("void pattern_transition"+std::to_string(pattern_index)+"_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(void);");
    output.add_source_line("void next_states_iterator::pattern_transition"+std::to_string(pattern_index)+"_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(void){");
    std::string pusher = "pattern_push"+std::to_string(pattern_index)+"()";
    std::string level_getter = "pattern_get_level"+std::to_string(pattern_index)+"()";
    std::string level_reverter = "pattern_revert_to_level"+std::to_string(pattern_index);
    actions_compiler ac(output,pieces_to_id,edges_to_id,variables_to_id,decl,pusher,level_getter,level_reverter,true);
    std::string human_readable_labels;
    std::string decision_stack = "pattern_decision_points"+std::to_string(pattern_index);
    int last_state_to_check = -1;
    handle_labels(output,ac,shift_tables,precomputed_patterns,decision_stack,human_readable_labels);
    uint current_state = local_register_endpoint_index;
    while(local_register[current_state].is_no_choicer(shift_tables)){
        if(local_register[current_state].can_be_checked_for_visit()){
            if(local_register[current_state].get_only_exit().label_list.empty())
                last_state_to_check = current_state;
            else{
                visit_node_in_pattern(output,current_state,pattern_index,ac);
                last_state_to_check = -1;
            }
        }
        else if(not local_register[current_state].get_only_exit().label_list.empty() and last_state_to_check >= 0){
            visit_node_in_pattern(output,last_state_to_check,pattern_index,ac);
            last_state_to_check = -1;
        }
        local_register[current_state].get_only_exit().handle_labels(output,ac,shift_tables,precomputed_patterns,decision_stack,human_readable_labels);
        current_state = local_register[current_state].get_only_exit().local_register_endpoint_index;
    }
    if(local_register[current_state].can_be_checked_for_visit())
        last_state_to_check = current_state;
    if(last_state_to_check >= 0)
        visit_node_in_pattern(output,current_state,pattern_index,ac);
    ac.finallize();
    assert(not ac.is_ready_to_report());
    if(local_register[current_state].is_dead_end()){
        output.add_source_line("success_to_report"+std::to_string(pattern_index)+" = true;");
        output.add_source_line("revert_to_point_after_pattern(pattern_decision_points"+std::to_string(pattern_index)+"[0], &resettable_bitarray_stack::pattern_revert_to_level"+std::to_string(pattern_index)+");");
        output.add_source_line("pattern_decision_points"+std::to_string(pattern_index)+".clear();");
    }
    else
        output.add_source_line("pattern_decision_points"+std::to_string(pattern_index)+".emplace_back(0,"+(opts.enabled_shift_tables()?"0,":std::string())+std::to_string(current_state)+",state_to_change.current_cell,cache.pattern_get_level"+std::to_string(pattern_index)+"(),board_change_points.size(),variables_change_points.size());");
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
