#include"automaton.hpp"

#include<cassert>
#include"cpp_container.hpp"

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
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl)const{
    for(uint i=0;i<local_register.size();++i)
        local_register[i].print_transition_functions(i,output,pieces_to_id,edges_to_id,variables_to_id,decl,local_register);
}

void automaton::print_transition_functions_inside_pattern(
    uint pattern_index,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl)const{
    for(uint i=0;i<local_register.size();++i)
        local_register[i].print_transition_functions_inside_pattern(i,pattern_index,output,pieces_to_id,edges_to_id,variables_to_id,decl,local_register);
}

void automaton::print_transition_table(cpp_container& output, const std::string& table_name, const std::string& functions_prefix)const{
    output.add_header_include("vector");
    output.add_header_line("const static std::vector<transition_function> "+table_name+"["+std::to_string(local_register.size())+"];");
    output.add_source_line("const std::vector<next_states_iterator::transition_function> next_states_iterator::"+table_name+"["+std::to_string(local_register.size())+"] = {");
    for(uint i=0;i<local_register.size();++i)
        local_register[i].print_outgoing_transitions(i,output, functions_prefix);
    output.add_source_line("};");
}

void automaton::mark_end_as_outgoing_usable(void){
    local_register[accept_state].mark_explicitly_as_transition_start();
}

void automaton::mark_start_as_outgoing_usable(void){
    local_register[start_state].mark_explicitly_as_transition_start();
}

void automaton::mark_states_as_double_reachable(void){
    std::vector<uint> reachability;
    reachability.resize(local_register.size());
    for(const auto& el: local_register)
        el.notify_endpoints_about_being_reachable(reachability);
    for(uint i=0;i<reachability.size();++i)
        if(reachability[i]>1)
            local_register[i].mark_as_doubly_reachable();
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

automaton edge_automaton(const std::vector<label>& label_list){
    automaton result;
    auto result_endpoints = result.prepare_new_endpoints();
    result.local_register[result_endpoints.first].connect_with_state(result_endpoints.second, label_list);
    result.set_endpoints(result_endpoints);
    return result;
}

automaton edge_automaton(const rbg_parser::game_move* action_label){
    std::vector<label> label_list;
    label_list.push_back({action,action_label,0});
    return edge_automaton(label_list);
}

automaton edge_automaton(uint pattern_automaton_index, bool positive){
    std::vector<label> label_list;
    label_list.push_back({(positive?positive_pattern:negative_pattern),nullptr,pattern_automaton_index});
    return edge_automaton(label_list);
}
