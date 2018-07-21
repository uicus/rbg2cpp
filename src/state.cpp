#include"state.hpp"

#include<cassert>

uint state::next_free_id = 0;

state::state(void):
state_id(next_free_id++),
next_states(),
started_checks(),
ended_checks(){
}

state::state(const state& rhs):
state_id(next_free_id++),
next_states(rhs.next_states),
started_checks(rhs.started_checks),
ended_checks(rhs.ended_checks){
}

state& state::operator=(const state& rhs){
    if(this == &rhs)
        return *this;
    state_id = next_free_id++;
    next_states = rhs.next_states;
    started_checks = rhs.started_checks;
    ended_checks = rhs.ended_checks;
    return *this;
}

uint state::get_id(void)const{
    return state_id;
}

void state::inform_about_being_appended(uint shift_value){
    for(auto& el: next_states)
        el.shift(shift_value);
    for(auto& el: started_checks){
        el.accept_state+=shift_value;
        el.start_state+=shift_value;
    }
    std::set<move_check_endpoints_info> modified_enders;
    for(const auto& el: ended_checks)
        modified_enders.insert({el.start_state+shift_value, el.accept_state+shift_value, el.negated});
    std::swap(modified_enders, ended_checks);
}

void state::inform_about_state_deletion(uint deleted_index){
    for(auto& el: next_states)
        el.inform_abut_state_deletion(deleted_index);
    for(auto& el: started_checks){
        if(el.accept_state > deleted_index)
            --el.accept_state;
        if(el.start_state > deleted_index)
            --el.start_state;
    }
    std::set<move_check_endpoints_info> modified_enders;
    for(const auto& el: ended_checks)
        modified_enders.insert({
            (el.start_state > deleted_index ? el.start_state-1 : el.start_state),
            (el.accept_state > deleted_index ? el.accept_state-1 : el.accept_state),
            el.negated
        });
    std::swap(modified_enders, ended_checks);
}

void state::absorb(state&& rhs){
    assert(next_states.empty());
    assert(started_checks.empty());
    assert(rhs.ended_checks.empty());
    if(not rhs.next_states.empty()){
        next_states = std::move(rhs.next_states);
        started_checks = std::move(rhs.started_checks);
        rhs.next_states.clear();
        rhs.started_checks.clear();
    }
}

void state::connect_with_state(uint index_in_local_register, const std::vector<const rbg_parser::game_move*>& label_list, uint index_after_traversing){
    edge result_edge(index_in_local_register);
    for(const auto el: label_list)
        result_edge.add_another_action(el);
    result_edge.set_index(index_after_traversing);
    next_states.push_back(std::move(result_edge));
}

void state::set_as_check_begin(uint check_start, uint check_end, bool negated){
    started_checks.push_back({check_start, check_end, negated});
}

void state::set_as_check_end(uint check_start, uint check_end, bool negated){
    ended_checks.insert({check_start, check_end, negated});
}

void state::print_transition_functions(
    uint from_state,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl)const{
    for(const auto& el: next_states)
        el.print_transition_function(from_state, output, pieces_to_id, edges_to_id, variables_to_id, decl);
}
