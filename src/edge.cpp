#include"edge.hpp"
#include"game_move.hpp"
#include"cpp_container.hpp"
#include"actions_compiler.hpp"
#include"declarations.hpp"

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

void edge::print_transition_function(
    uint from_state,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl)const{
    output.add_header_line("void transition_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(void);");
    output.add_source_line("void next_states_iterator::transition_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(void){");
    actions_compiler ac(output,pieces_to_id,edges_to_id,variables_to_id,decl);
    for(const auto& el: label_list)
        el->accept(ac);
    ac.finallize();
    output.add_source_line("state_to_change.current_state = "+std::to_string(local_register_endpoint_index)+";");
    output.add_source_line("}");
    output.add_source_line("");
}
