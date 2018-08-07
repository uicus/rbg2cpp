#include"automaton_builder.hpp"
#include"game_move.hpp"
#include"state.hpp"
#include"edge.hpp"
#include"actions_block.hpp"
#include"arithmetic_comparison.hpp"
#include"move_check.hpp"
#include"shift.hpp"
#include"ons.hpp"
#include"offs.hpp"
#include"assignments.hpp"
#include"switch.hpp"
#include"star_move.hpp"
#include"concatenation.hpp"
#include"sum.hpp"

uint automaton_builder::current_index = 0;

automaton_builder::automaton_builder(std::vector<automaton>& pattern_automata, bool should_assign_indices):
result(),
should_assign_indices(should_assign_indices),
pattern_automata(pattern_automata),
block_building_mode(false),
block_has_switch(false),
current_block(){
}

void automaton_builder::dispatch(const rbg_parser::arithmetic_comparison& m){
    if(block_building_mode)
        current_block.push_back({action,&m,0});
    else
        result = edge_automaton(&m, should_assign_indices ? ++current_index : 0);
}

void automaton_builder::dispatch(const rbg_parser::move_check& m){
    automaton_builder element_builder(pattern_automata, false);
    m.get_content()->accept(element_builder);
    pattern_automata.push_back(element_builder.get_final_result());
    if(block_building_mode)
        current_block.push_back({(m.is_negated()?negative_pattern:positive_pattern),nullptr,uint(pattern_automata.size()-1)});
    else
        result = edge_automaton(pattern_automata.size()-1,not m.is_negated(), should_assign_indices ? ++current_index : 0);
}

void automaton_builder::dispatch(const rbg_parser::shift& m){
    if(block_building_mode)
        current_block.push_back({action,&m,0});
    else
        result = edge_automaton(&m, should_assign_indices ? ++current_index : 0);
}

void automaton_builder::dispatch(const rbg_parser::ons& m){
    if(block_building_mode)
        current_block.push_back({action,&m,0});
    else
        result = edge_automaton(&m, should_assign_indices ? ++current_index : 0);
}

void automaton_builder::dispatch(const rbg_parser::off& m){
    if(block_building_mode)
        current_block.push_back({action,&m,0});
    else
        result = edge_automaton(&m, should_assign_indices ? ++current_index : 0);
}

void automaton_builder::dispatch(const rbg_parser::assignment& m){
    if(block_building_mode)
        current_block.push_back({action,&m,0});
    else
        result = edge_automaton(&m, should_assign_indices ? ++current_index : 0);
}

void automaton_builder::dispatch(const rbg_parser::player_switch& m){
    if(block_building_mode){
        current_block.push_back({action,&m,0});
        block_has_switch = true;
    }
    else{
        result = edge_automaton(&m, should_assign_indices ? ++current_index : 0);
        result.mark_end_as_outgoing_usable();
    }
}

void automaton_builder::dispatch(const rbg_parser::keeper_switch& m){
    if(block_building_mode){
        current_block.push_back({action,&m,0});
        block_has_switch = true;
    }
    else{
        result = edge_automaton(&m, should_assign_indices ? ++current_index : 0);
        result.mark_end_as_outgoing_usable();
    }
}

void automaton_builder::dispatch(const rbg_parser::actions_block& m){
    automaton_builder elements_builder(pattern_automata,true);
    elements_builder.block_building_mode = true;
    for(const auto& el: m.get_content()){
        if(should_assign_indices)
            ++current_index;
        el->accept(elements_builder);
    }
    result = edge_automaton(elements_builder.current_block, current_index);
    if(block_has_switch)
        result.mark_end_as_outgoing_usable();
}

void automaton_builder::dispatch(const rbg_parser::star_move& m){
    automaton_builder element_builder(pattern_automata, should_assign_indices);
    m.get_content()->accept(element_builder);
    result = element_builder.get_final_result();
    result.starify_automaton();
}

void automaton_builder::dispatch(const rbg_parser::sum& m){
    std::vector<automaton> elements;
    for(const auto& el: m.get_content()){
        automaton_builder element_builder(pattern_automata, should_assign_indices);
        el->accept(element_builder);
        elements.push_back(element_builder.get_final_result());
    }
    result = sum_of_automatons(std::move(elements));
}

void automaton_builder::dispatch(const rbg_parser::concatenation& m){
    std::vector<automaton> elements;
    for(const auto& el: m.get_content()){
        automaton_builder element_builder(pattern_automata, should_assign_indices);
        el->accept(element_builder);
        elements.push_back(element_builder.get_final_result());
    }
    result = concatenation_of_automatons(std::move(elements));
}

automaton automaton_builder::get_final_result(void){
    return std::move(result);
}
