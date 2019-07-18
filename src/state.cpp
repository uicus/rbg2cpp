#include"state.hpp"
#include"compiler_options.hpp"
#include"cpp_container.hpp"
#include"actions_compiler.hpp"
#include"transition_data.hpp"
#include<cassert>

void state::inform_about_being_appended(uint shift_value){
    for(auto& el: next_states)
        el.shift(shift_value);
}

void state::inform_about_state_deletion(uint deleted_index){
    for(auto& el: next_states)
        el.inform_abut_state_deletion(deleted_index);
}

void state::absorb(state&& rhs){
    assert(next_states.empty());
    if(not rhs.next_states.empty()){
        next_states = std::move(rhs.next_states);
        outgoing_edges_needed |= rhs.outgoing_edges_needed;
        doubly_reachable |= rhs.doubly_reachable;
        assert(rhs.state_user == none);
        rhs.next_states.clear();
    }
}

void state::connect_with_state(uint index_in_local_register, const std::vector<label>& label_list){
    next_states.push_back(edge(index_in_local_register, label_list));
}

void state::print_transition_functions(
    uint from_state,
    cpp_container& output,
    const static_transition_data& static_data,
    const std::vector<state>& local_register)const{
    if(next_states.size()>1 or outgoing_edges_needed)
        for(const auto& el: next_states){
            dynamic_transition_data dynamic_data(static_data,from_state);
            el.print_transition_function(output, static_data, dynamic_data, local_register);
        }
}

void state::print_outgoing_all_transitions(uint from_state, cpp_container& output, const std::string& functions_prefix)const{
    std::string resulting_line = "{";
    if(next_states.size()>1 or outgoing_edges_needed){
        output.add_source_line("case "+std::to_string(from_state)+":");
        for(uint i=0;i<next_states.size();++i)
            output.add_source_line(functions_prefix+"_"+std::to_string(from_state)+"_"+std::to_string(next_states[i].get_endpoint())+"(current_cell, cache, mr, moves);");
        output.add_source_line("break;");
    }
}

void state::print_outgoing_any_transitions(uint from_state, cpp_container& output, const std::string& functions_prefix)const{
    std::string resulting_line = "{";
    if(next_states.size()>1 or outgoing_edges_needed){
        output.add_source_line("case "+std::to_string(from_state)+":");
        for(uint i=0;i<next_states.size();++i){
            output.add_source_line("if("+functions_prefix+"_"+std::to_string(from_state)+"_"+std::to_string(next_states[i].get_endpoint())+"(current_cell, cache)){");
            output.add_source_line("return true;");
            output.add_source_line("}");
        }
        output.add_source_line("return false;");
    }
}

void state::notify_endpoints_about_being_reachable(std::vector<uint>& reachability, const std::vector<shift_table>& shift_tables)const{
    for(const auto& el: next_states){
        if(el.is_shift_table_with_multiple_choices(shift_tables))
            ++reachability[el.get_endpoint()];
        ++reachability[el.get_endpoint()];
    }
}

void state::mark_as_keeper_move_beginning(void){
    state_user = keeper_start;
}

void state::mark_as_player_move_beginning(void){
    state_user = player_start;
}

void state::mark_as_doubly_reachable(void){
    doubly_reachable = true;
}

bool state::can_be_checked_for_visit(void)const{
    return doubly_reachable;
}

void state::mark_explicitly_as_transition_start(void){
    outgoing_edges_needed = true;
}

const edge& state::get_only_exit(void)const{
    return next_states[0];
}

bool state::is_dead_end(void)const{
    return next_states.empty();
}

bool state::is_no_choicer(void)const{
    return next_states.size() == 1;
}

void state::push_next_states_to_shift_tables_dfs_stack(
    uint current_cell,
    const rbg_parser::graph& board,
    std::vector<std::pair<uint,uint>>& dfs_stack,
    const std::vector<precomputed_pattern>& pps)const{
    for(const auto& el: next_states){
        auto cell = el.get_next_cell(current_cell,board,pps);
        if(cell>=0)
            dfs_stack.emplace_back(el.get_endpoint(), uint(cell));
    }
}

void state::print_recursive_calls(
    uint from_state,
    cpp_container& output,
    const static_transition_data& static_data,
    dynamic_transition_data& dynamic_data,
    const std::string& cell)const{
    switch(static_data.kind){
        case all_getter:
            for(uint i=0;i<next_states.size();++i){
                output.add_source_line(static_data.name_prefix+std::to_string(from_state)+"_"+std::to_string(next_states[i].get_endpoint())+"("+cell+", cache, mr, moves);");
            }
            break;
        case any_getter:
        case inside_pattern:
            for(uint i=0;i<next_states.size();++i){
                output.add_source_line("if("+static_data.name_prefix+std::to_string(from_state)+"_"+std::to_string(next_states[i].get_endpoint())+"("+cell+", cache)){");
                dynamic_data.insert_reverting_sequence_after_success(output);
                output.add_source_line("}");
            }
            break;
    }
}

void state::print_indices_to_actions_correspondence(
    cpp_container& output,
    const static_transition_data& static_data)const{
    for(const auto& el: next_states)
        el.print_indices_to_actions_correspondence(output, static_data);
}
