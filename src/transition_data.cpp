#include"transition_data.hpp"
#include"token.hpp"
#include"declarations.hpp"
#include"shift_table.hpp"
#include"precomputed_pattern.hpp"
#include"cpp_container.hpp"
#include"state.hpp"
#include"cache_checks_container.hpp"
#include"compiler_options.hpp"
#include<cassert>

static_transition_data::static_transition_data(
    const compiler_options& opts,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::vector<shift_table>& shift_tables,
    const std::vector<precomputed_pattern>& precomputed_patterns,
    const cache_checks_container& ccc,
    bool uses_pieces_in_arithmetics,
    bool injective_board,
    const std::string& name_prefix,
    kind_of_transition kind,
    uint pattern_index)
      : opts(opts),
        pieces_to_id(pieces_to_id),
        edges_to_id(edges_to_id),
        variables_to_id(variables_to_id),
        decl(decl),
        shift_tables(shift_tables),
        precomputed_patterns(precomputed_patterns),
        ccc(ccc),
        uses_pieces_in_arithmetics(uses_pieces_in_arithmetics),
        injective_board(injective_board),
        return_type(),
        name_prefix(name_prefix),
        success_finish(),
        failure_finish(),
        cache_level_getter(),
        cache_level_pusher(),
        cache_level_reverter(),
        cache_setter(),
        cache_set_getter(),
        kind(kind),
        pattern_index(pattern_index){
    switch(kind){
        case all_getter:
            return_type = "void";
            success_finish = failure_finish = "return;";
            break;
        case any_getter:
        case inside_pattern:
            return_type = "bool";
            success_finish = "return true;";
            failure_finish = "return false;";
            break;
    }
    switch(kind){
        case all_getter:
        case any_getter:
            cache_level_getter = "cache.get_level();";
            cache_level_pusher = "cache.push();";
            cache_level_reverter = "cache.revert_to_level(previous_cache_level);";
            cache_setter = "cache.set";
            cache_set_getter = "cache.is_set";
            break;
        case inside_pattern:
            cache_level_getter = "cache.pattern_get_level"+std::to_string(pattern_index)+"();";
            cache_level_pusher = "cache.pattern_push"+std::to_string(pattern_index)+"();";
            cache_level_reverter = "cache.pattern_revert_to_level"+std::to_string(pattern_index)+"(previous_cache_level);";
            cache_setter = "cache.pattern_set"+std::to_string(pattern_index);
            cache_set_getter = "cache.pattern_is_set"+std::to_string(pattern_index);
            break;
    }

}

dynamic_transition_data::dynamic_transition_data(const static_transition_data& static_data, uint from_state):
static_data(static_data),
reverting_stack(),
encountered_any_change(false),
should_check_cell_correctness(false),
pending_modifier(false),
has_saved_cache_level(false),
next_player(-1),
from_state(std::to_string(from_state)),
last_state_to_check(-1),
branching_shift_table_to_handle(-1){
}

bool dynamic_transition_data::should_use_cache(void)const{
    return (static_data.kind == inside_pattern and static_data.ccc.is_pattern_cache_needed(static_data.pattern_index))
        or (static_data.kind != inside_pattern and static_data.ccc.is_main_cache_needed());
}

const std::string& dynamic_transition_data::get_start_state(void)const{
    return from_state;
}

void dynamic_transition_data::save_board_change_for_later_revert(cpp_container& output, uint piece_id){
    output.add_source_line(std::string(static_data.kind == any_getter ? "[[maybe_unused]] ":"")+"int board_change"+std::to_string(reverting_stack.size())+"_cell = cell;");
    output.add_source_line(std::string(static_data.kind == any_getter ? "[[maybe_unused]] ":"")+"int board_change"+std::to_string(reverting_stack.size())+"_piece = pieces[cell];");
    reverting_stack.push_back({board_change,piece_id});
    pending_modifier = true;
}

void dynamic_transition_data::save_variable_change_for_later_revert(cpp_container& output, uint variable_id){
    output.add_source_line(std::string(static_data.kind == any_getter ? "[[maybe_unused]] ":"")+"int variable_change"+std::to_string(reverting_stack.size())+" = variables["+std::to_string(variable_id)+"];");
    reverting_stack.push_back({variable_change,variable_id});
    pending_modifier = true;
}

void dynamic_transition_data::revert_board_change(cpp_container& output, uint piece_id, uint stack_position)const{
    if(static_data.uses_pieces_in_arithmetics){
        output.add_source_line("--pieces_count["+std::to_string(piece_id)+"];");
        output.add_source_line("++pieces_count[board_change"+std::to_string(stack_position)+"_piece];");
    }
    output.add_source_line("pieces[board_change"+std::to_string(stack_position)+"_cell] = board_change"+std::to_string(stack_position)+"_piece;");
}

void dynamic_transition_data::revert_variable_change(cpp_container& output, uint variable_id, uint stack_position)const{
    output.add_source_line("variables["+std::to_string(variable_id)+"] = variable_change"+std::to_string(stack_position)+";");
}

void dynamic_transition_data::insert_move_size_check(cpp_container& output, uint state_index)const{
    if(static_data.kind == all_getter and static_data.opts.enabled_semi_split_generation() and encountered_any_change){
        output.add_source_line("if(mr.size()>=move_length_limit){");
        output.add_source_line("moves.emplace_back(mr, cell, "+std::to_string(state_index)+");");
        insert_reverting_sequence_after_success(output);
        output.add_source_line("}");
    }
}

void dynamic_transition_data::push_any_change_on_modifiers_list(cpp_container& output, const std::string& index, const std::string& cell){
    if(static_data.kind == all_getter){
        if(not encountered_any_change)
            output.add_source_line("const auto previous_changes_list = mr.size();");
        output.add_source_line("mr.emplace_back("+index+","+cell+");");
        encountered_any_change = true;
    }
}

void dynamic_transition_data::print_modifiers_applications_revert(cpp_container& output)const{
    for(uint i=reverting_stack.size();i>0;--i)
        switch(reverting_stack[i-1].type){
            case board_change:
                revert_board_change(output, reverting_stack[i-1].additional_info, i-1);
                break;
            case variable_change:
                revert_variable_change(output, reverting_stack[i-1].additional_info, i-1);
                break;
        }
}

void dynamic_transition_data::print_cache_level_revert(cpp_container& output)const{
    if(has_saved_cache_level)
        output.add_source_line(static_data.cache_level_reverter);
}

void dynamic_transition_data::print_modifiers_list_revert(cpp_container& output)const{
    if(static_data.kind != inside_pattern)
        if(encountered_any_change)
            output.add_source_line("mr.resize(previous_changes_list);");
}

void dynamic_transition_data::insert_unended_reverting_sequence(cpp_container& output)const{
    print_cache_level_revert(output);
    print_modifiers_list_revert(output);
    print_modifiers_applications_revert(output);
}

void dynamic_transition_data::insert_reverting_sequence_after_fail(cpp_container& output)const{
    insert_unended_reverting_sequence(output);
    output.add_source_line(static_data.failure_finish);
}

void dynamic_transition_data::insert_reverting_sequence_after_success(cpp_container& output)const{
    if(static_data.kind != any_getter)
        insert_unended_reverting_sequence(output);
    output.add_source_line(static_data.success_finish);
}

void dynamic_transition_data::handle_waiting_modifier(cpp_container& output){
    if(pending_modifier and should_use_cache()){
        if(not has_saved_cache_level)
            output.add_source_line(std::string(static_data.kind == any_getter ? "[[maybe_unused]] ":"")+"unsigned int previous_cache_level = "+static_data.cache_level_getter);
        has_saved_cache_level = true;
        output.add_source_line(static_data.cache_level_pusher);
    }
    pending_modifier = false;
}

void dynamic_transition_data::handle_cell_check(cpp_container& output){
    if(should_check_cell_correctness){
        should_check_cell_correctness = false;
        if(not static_data.injective_board)
            visit_node(output);
        output.add_source_line("if(cell == 0){");
        insert_reverting_sequence_after_fail(output);
        output.add_source_line("}");
    }
}

void dynamic_transition_data::queue_cell_check(void){
    should_check_cell_correctness = true;
}

void dynamic_transition_data::clear_queue_checks(void){
    should_check_cell_correctness = false;
}

void dynamic_transition_data::set_next_player(uint next){
    next_player = next;
}

bool dynamic_transition_data::is_ready_to_report(void)const{
    return next_player >= 0;
}

int dynamic_transition_data::get_next_player(void)const{
    assert(is_ready_to_report());
    return next_player;
}

void dynamic_transition_data::finallize(cpp_container& output){
    handle_cell_check(output);
    handle_waiting_modifier(output);
}

void dynamic_transition_data::queue_state_to_check_visited(uint state_index){
    if(static_data.kind == inside_pattern){
        if(static_data.ccc.should_cache_be_checked_in_pattern(state_index, static_data.pattern_index))
            last_state_to_check = static_data.ccc.get_cache_cell_to_check_in_pattern(state_index, static_data.pattern_index);
    }
    else if(static_data.ccc.should_cache_be_checked(state_index))
        last_state_to_check = static_data.ccc.get_cache_cell_to_check(state_index);
}

bool dynamic_transition_data::should_check_for_visited(void)const{
    return last_state_to_check >= 0;
}

void dynamic_transition_data::visit_node(
    cpp_container& output,
    const std::string& cell,
    bool custom_fail_instruction,
    const std::string& fail_instruction){
    finallize(output);
    if(should_check_for_visited()){
        output.add_source_line("if("+static_data.cache_set_getter+"("+std::to_string(last_state_to_check)+", "+cell+"-1)){");
        if(custom_fail_instruction)
            output.add_source_line(fail_instruction);
        else
            insert_reverting_sequence_after_fail(output);
        output.add_source_line("}");
        output.add_source_line(static_data.cache_setter+"("+std::to_string(last_state_to_check)+", "+cell+"-1);");
        last_state_to_check = -1;
    }
}

void dynamic_transition_data::queue_branching_shift_table(uint index){
    branching_shift_table_to_handle = index;
}

bool dynamic_transition_data::should_handle_branching_shift_table(void)const{
    return branching_shift_table_to_handle >= 0;
}

void dynamic_transition_data::handle_branching_shift_table(cpp_container& output, const state& state_at_end, uint state_index){
    assert(should_handle_branching_shift_table());
    finallize(output);
    if(static_data.shift_tables[branching_shift_table_to_handle].is_any_square())
        output.add_source_line("for(int el=1;el<"+std::to_string(static_data.shift_tables[branching_shift_table_to_handle].get_size()+1)+";++el){");
    else
        output.add_source_line("for(const auto el: shift_table"+std::to_string(branching_shift_table_to_handle)+"[cell]){");
    queue_state_to_check_visited(state_index);
    visit_node(output,"el",true,"continue;");
    state_at_end.print_recursive_calls(state_index,output,static_data,*this,"el");
    output.add_source_line("}");
    insert_reverting_sequence_after_fail(output);
}

void dynamic_transition_data::handle_standard_transition_end(cpp_container& output, const state& state_at_end, uint state_index){
    if(state_at_end.can_be_checked_for_visit())
        queue_state_to_check_visited(state_index);
    if(should_check_for_visited())
        visit_node(output);
    if(static_data.kind == inside_pattern and state_at_end.is_dead_end()){
        handle_cell_check(output);
        insert_reverting_sequence_after_success(output);
    }
    else if(static_data.kind != inside_pattern and is_ready_to_report()){
        handle_cell_check(output);
        if(static_data.kind == all_getter){
            if(static_data.opts.enabled_semi_split_generation())
                output.add_source_line("moves.emplace_back(mr, cell, "+std::to_string(state_index)+");");
            else
                output.add_source_line("moves.emplace_back(mr);");
        }
        else{
            output.add_source_line("current_player = "+std::to_string(next_player)+";");
            output.add_source_line("current_state = "+std::to_string(state_index)+";");
            output.add_source_line("current_cell = cell;");
        }
        insert_reverting_sequence_after_success(output);
    }
    else{
        insert_move_size_check(output, state_index);
        finallize(output);
        state_at_end.print_recursive_calls(state_index,output,static_data,*this);
        insert_reverting_sequence_after_fail(output);
    }
}

bool dynamic_transition_data::can_handle_further_labels(void)const{
    return not is_ready_to_report() and not should_handle_branching_shift_table();
}
