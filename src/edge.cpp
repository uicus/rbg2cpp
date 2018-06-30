#include"edge.hpp"
#include"game_move.hpp"

edge::edge(uint local_register_endpoint_index):
local_register_endpoint_index(local_register_endpoint_index),
label_list(),
index_after_traversing(0){
}

void edge::add_another_action(const rbg_parser::game_move* action){
    label_list.push_back(action);
}

void edge::shift(uint shift_value){
    local_register_endpoint_index += shift_value;
}

void edge::inform_abut_state_deletion(uint deleted_index){
    if(local_register_endpoint_index>deleted_index)
        --local_register_endpoint_index;
}

void edge::set_index(uint index){
    index_after_traversing = index;
}
