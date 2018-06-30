#include"state.hpp"

#include<cassert>

uint state::next_free_id = 0;

state::state(void):
state_id(next_free_id++),
next_states(){
}

state::state(const state& rhs):
state_id(next_free_id++),
next_states(rhs.next_states){
}

state& state::operator=(const state& rhs){
    if(this == &rhs)
        return *this;
    state_id = next_free_id++;
    next_states = rhs.next_states;
    return *this;
}

uint state::get_id(void)const{
    return state_id;
}

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
        rhs.next_states.clear();
    }
}

void state::connect_with_state(uint index_in_local_register, const std::vector<const rbg_parser::game_move*>& label_list, uint index_after_traversing){
    edge result_edge(index_in_local_register);
    for(const auto el: label_list)
        result_edge.add_another_action(el);
    result_edge.set_index(index_after_traversing);
    next_states.push_back(std::move(result_edge));
}
