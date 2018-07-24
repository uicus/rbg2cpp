#include"edge.hpp"
#include"game_move.hpp"
#include"cpp_container.hpp"
#include"actions_compiler.hpp"
#include"declarations.hpp"
#include"state.hpp"

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

uint edge::get_endpoint(void)const{
    return local_register_endpoint_index;
}

void edge::print_transition_function(
    uint from_state,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::vector<state>& local_register)const{
    output.add_header_line("void transition_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(void);");
    output.add_source_line("void next_states_iterator::transition_"+std::to_string(from_state)+"_"+std::to_string(local_register_endpoint_index)+"(void){");
    if(local_register[local_register_endpoint_index].is_dead_end()){
        output.add_source_line("revert_to_last_choice();");
    }
    else{
        actions_compiler ac(output,pieces_to_id,edges_to_id,variables_to_id,decl);
        for(const auto& el: label_list)
            el->accept(ac);
        ac.finallize();
        output.add_source_line("state_to_change.current_state = "+std::to_string(local_register_endpoint_index)+";");
        output.add_source_line("if(cache.is_set(state_to_change.current_state, state_to_change.current_cell)){");
        output.add_source_line("revert_to_last_choice();");
        output.add_source_line("return;");
        output.add_source_line("}");
        output.add_source_line("cache.set(state_to_change.current_state, state_to_change.current_cell);");
        if(ac.is_ready_to_report())
            output.add_source_line("ready_to_report = true;");
        else{
            if(local_register[local_register_endpoint_index].is_no_choicer())
                output.add_source_line("(this->*transitions[state_to_change.current_state][0])();");
            else
                output.add_source_line("decision_points.push_back({0,state_to_change.current_state,state_to_change.current_cell,cache.get_level(),board_change_points.size(),variables_change_points.size()});");
        }
    }
    output.add_source_line("}");
    output.add_source_line("");
}
