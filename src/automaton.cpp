#include"automaton.hpp"
#include"compiler_options.hpp"
#include"cpp_container.hpp"
#include"shift_table.hpp"
#include"graph.hpp"
#include<cassert>

uint automaton::get_start_state(void){
    return start_state;
}

uint automaton::get_size(void){
    return local_register.size();
}

std::pair<uint,uint> automaton::place_side_by_side(automaton&& rhs){
    for(auto& el: rhs.local_register)
        el.inform_about_being_appended(local_register.size());
    uint appendee_start = local_register.size() + rhs.start_state;
    uint appendee_accept = local_register.size() + rhs.accept_state;
    local_register.reserve(local_register.size() + rhs.local_register.size());
    std::move(std::begin(rhs.local_register), std::end(rhs.local_register), std::back_inserter(local_register));
    rhs.local_register.clear();
    return std::make_pair(appendee_start,appendee_accept);
}


std::pair<uint,uint> automaton::prepare_new_endpoints(void){
    uint new_start_state = local_register.size();
    local_register.push_back(state());
    uint new_accept_state = local_register.size();
    local_register.push_back(state());
    return std::make_pair(new_start_state,new_accept_state);
}

void automaton::set_endpoints(const std::pair<uint,uint>& new_endpoints){
    start_state = new_endpoints.first;
    accept_state = new_endpoints.second;
}

void automaton::concat_automaton(automaton&& concatee){
    uint old_start = concatee.start_state;
    for(auto& el: concatee.local_register){
        el.inform_about_state_deletion(old_start);
        el.inform_about_being_appended(local_register.size());
    }
    local_register[accept_state].absorb(std::move(concatee.local_register[old_start]));
    if(concatee.accept_state>concatee.start_state)
        accept_state = concatee.accept_state+local_register.size()-1;
    else if(concatee.accept_state<concatee.start_state)
        accept_state = concatee.accept_state+local_register.size();
    local_register.reserve(local_register.size() + concatee.local_register.size()-1);
    std::move(std::begin(concatee.local_register), std::begin(concatee.local_register)+old_start, std::back_inserter(local_register));
    std::move(std::begin(concatee.local_register)+old_start+1, std::end(concatee.local_register), std::back_inserter(local_register));
}

void automaton::starify_automaton(void){
    auto new_endpoints = prepare_new_endpoints();
    local_register[new_endpoints.first].connect_with_state(start_state);
    local_register[accept_state].connect_with_state(new_endpoints.second);
    local_register[new_endpoints.first].connect_with_state(new_endpoints.second);
    local_register[accept_state].connect_with_state(start_state);
    set_endpoints(new_endpoints);
}

void automaton::print_transition_functions(
    cpp_container& output,
    const static_transition_data& static_data)const{
    for(uint i=0;i<local_register.size();++i)
        local_register[i].print_transition_functions(i,output,static_data,local_register);
}

void automaton::print_all_getters_table(cpp_container& output, const std::string& functions_prefix)const{
    output.add_source_line("switch(state){");
    for(uint i=0;i<local_register.size();++i)
        local_register[i].print_outgoing_all_transitions(i,output,functions_prefix);
    output.add_source_line("default:");
    output.add_source_line("break;");
    output.add_source_line("}");
}

void automaton::print_any_appliers_table(cpp_container& output, const std::string& functions_prefix)const{
    output.add_source_line("switch(state){");
    for(uint i=0;i<local_register.size();++i)
        local_register[i].print_outgoing_any_transitions(i,output,functions_prefix);
    output.add_source_line("default:");
    output.add_source_line("return false;");
    output.add_source_line("}");
}

void automaton::mark_end_as_move_end(void){
    local_register[accept_state].mark_as_move_ender();
    mark_end_as_outgoing_usable();
}

void automaton::mark_end_as_outgoing_usable(void){
    local_register[accept_state].mark_explicitly_as_transition_start();
}

void automaton::mark_start_as_outgoing_usable(void){
    local_register[start_state].mark_explicitly_as_transition_start();
}

void automaton::mark_states_as_double_reachable(const std::vector<shift_table>& shift_tables){
    std::vector<uint> reachability;
    reachability.resize(local_register.size());
    for(const auto& el: local_register)
        el.notify_endpoints_about_being_reachable(reachability, shift_tables);
    for(uint i=0;i<reachability.size();++i)
        if(reachability[i]>1)
            local_register[i].mark_as_doubly_reachable();
}

void automaton::mark_connections_to_reachable_states(
    uint source_cell,
    const rbg_parser::graph& board,
    shift_table& table_to_modify,
    const std::vector<precomputed_pattern>& pps)const{
    std::vector<std::vector<bool>> visited(local_register.size());
    for(uint i=0;i<visited.size();++i)
        visited[i].resize(board.get_size());
    std::vector<std::pair<uint,uint>> dfs_stack;
    dfs_stack.emplace_back(start_state,source_cell);
    while(not dfs_stack.empty()){
        auto next_node = dfs_stack.back();
        dfs_stack.pop_back();
        if(not visited[next_node.first][next_node.second]){
            visited[next_node.first][next_node.second] = true;
            if(next_node.first == accept_state)
                table_to_modify.report_connection(source_cell, next_node.second);
            else
                local_register[next_node.first].push_next_states_to_shift_tables_dfs_stack(next_node.second,board,dfs_stack,pps);
        }
    }
}

shift_table automaton::generate_shift_table(
    const rbg_parser::graph& board,
    const std::vector<precomputed_pattern>& pps)const{
    shift_table result(board.get_size());
    for(uint i=0;i<board.get_size();++i)
        mark_connections_to_reachable_states(i,board,result,pps);
    return result;
}

void automaton::print_recursive_calls_for_pattern_in_start_state(
    cpp_container& output,
    const static_transition_data& static_data)const{
    local_register[start_state].print_recursive_calls(start_state,output,static_data);
}

void automaton::add_information_about_states_to_see(std::map<uint,uint>& states_to_bool_array)const{
    for(const auto& el: local_register)
        el.add_information_about_state_to_see(states_to_bool_array);
}

void automaton::see_what_states_must_be_marked_by_move_enders(void){
    std::vector<uint> states_to_mark_if_end;
    std::vector<bool> visited_states(local_register.size(),false);
    local_register[start_state].run_dfs_to_get_states_to_mark(start_state,states_to_mark_if_end,visited_states,local_register);
}

void automaton::print_indices_to_actions_correspondence(
    cpp_container& output,
    const static_transition_data& static_data)const{
    for(const auto& el: local_register)
        el.print_indices_to_actions_correspondence(output, static_data);
}

automaton concatenation_of_automatons(std::vector<automaton>&& elements){
    assert(not elements.empty());
    auto result = std::move(elements[0]);
    for(uint i=1;i<elements.size();++i)
        result.concat_automaton(std::move(elements[i]));
    elements.clear();
    return result;
}

automaton sum_of_automatons(std::vector<automaton>&& elements){
    automaton result;
    auto result_endpoints = result.prepare_new_endpoints();
    result.set_endpoints(result_endpoints);
    for(auto& el: elements){
        auto old_endpoints = result.place_side_by_side(std::move(el));
        result.local_register[result_endpoints.first].connect_with_state(old_endpoints.first);
        result.local_register[old_endpoints.second].connect_with_state(result_endpoints.second);
    }
    elements.clear();
    return result;
}

automaton prioritized_sum_of_automatons(std::vector<automaton>&& elements){
    automaton result = sum_of_automatons(std::move(elements));
    result.local_register[result.start_state].set_state_to_see_before_continuing(result.accept_state);
    return result;
}

automaton edge_automaton(const std::vector<label>& label_list){
    automaton result;
    auto result_endpoints = result.prepare_new_endpoints();
    result.local_register[result_endpoints.first].connect_with_state(result_endpoints.second, label_list);
    result.set_endpoints(result_endpoints);
    return result;
}
