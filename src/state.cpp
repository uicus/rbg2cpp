#include"state.hpp"
#include"edge.hpp"

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
