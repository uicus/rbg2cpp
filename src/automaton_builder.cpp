#include"automaton_builder.hpp"
#include"game_move.hpp"
#include"state.hpp"
#include"edge.hpp"
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

automaton_builder::automaton_builder(std::vector<automaton>& pattern_automata, std::vector<label>& current_block):
local_copy_automaton(),
currently_modified_automaton(local_copy_automaton),
has_automaton(false),
pattern_automata(pattern_automata),
current_block(current_block){
}

automaton_builder::automaton_builder(std::vector<automaton>& pattern_automata, std::vector<label>& current_block, automaton& upper_level_automaton):
local_copy_automaton(),
currently_modified_automaton(upper_level_automaton),
has_automaton(false),
pattern_automata(pattern_automata),
current_block(current_block){
}

automaton_builder automaton_builder::delegate_builder(){
    automaton_builder b(pattern_automata, current_block, currently_modified_automaton);
    b.has_automaton = has_automaton;
    return b;
}

void automaton_builder::dispatch(const rbg_parser::arithmetic_comparison& m){
    current_block.push_back({action,&m,0});
}

void automaton_builder::dispatch(const rbg_parser::move_check& m){
    std::vector<label> block;
    automaton_builder element_builder(pattern_automata, block);
    m.get_content()->accept(element_builder);
    pattern_automata.push_back(element_builder.get_final_result());
    current_block.push_back({(m.is_negated()?negative_pattern:positive_pattern),nullptr,uint(pattern_automata.size()-1)});
}

void automaton_builder::dispatch(const rbg_parser::shift& m){
    current_block.push_back({action,&m,0});
}

void automaton_builder::dispatch(const rbg_parser::ons& m){
    current_block.push_back({action,&m,0});
}

void automaton_builder::dispatch(const rbg_parser::off& m){
    current_block.push_back({action,&m,0});
}

void automaton_builder::dispatch(const rbg_parser::assignment& m){
    current_block.push_back({action,&m,0});
}

void automaton_builder::build_automaton_from_actions_so_far(){
    if(not current_block.empty())
        concat_automaton_to_result_so_far(edge_automaton(current_block));
    current_block.clear();
}

void automaton_builder::concat_automaton_to_result_so_far(automaton&& a){
    if(has_automaton)
        currently_modified_automaton.concat_automaton(std::move(a));
    else
        currently_modified_automaton = std::move(a);
    has_automaton = true;
}

void automaton_builder::handle_any_switch(const rbg_parser::game_move& m){
    current_block.push_back({action,&m,0});
    build_automaton_from_actions_so_far();
    currently_modified_automaton.mark_end_as_outgoing_usable();
}

void automaton_builder::dispatch(const rbg_parser::player_switch& m){
    handle_any_switch(m);
}

void automaton_builder::dispatch(const rbg_parser::keeper_switch& m){
    handle_any_switch(m);
}

void automaton_builder::dispatch(const rbg_parser::star_move& m){
    build_automaton_from_actions_so_far();
    std::vector<label> block;
    automaton_builder element_builder(pattern_automata,block);
    m.get_content()->accept(element_builder);
    auto temp_automaton = element_builder.get_final_result();
    assert(block.empty());
    temp_automaton.starify_automaton();
    concat_automaton_to_result_so_far(std::move(temp_automaton));
}

void automaton_builder::dispatch(const rbg_parser::sum& m){
    build_automaton_from_actions_so_far();
    std::vector<automaton> elements;
    for(const auto& el: m.get_content()){
        std::vector<label> block;
        automaton_builder element_builder(pattern_automata, block);
        el->accept(element_builder);
        elements.push_back(element_builder.get_final_result());
        assert(block.empty());
    }
    concat_automaton_to_result_so_far(sum_of_automatons(std::move(elements)));
}

void automaton_builder::dispatch(const rbg_parser::concatenation& m){
    auto b(delegate_builder());
    for(const auto& el: m.get_content())
        el->accept(b);
    has_automaton = b.has_automaton;
}

automaton automaton_builder::get_final_result(void){
    if(has_automaton){
        if(not current_block.empty())
            currently_modified_automaton.concat_automaton(edge_automaton(current_block));
        current_block.clear();
        return std::move(currently_modified_automaton);
    }
    else{
        auto result = edge_automaton(current_block);
        current_block.clear();
        return result;
    }
}
