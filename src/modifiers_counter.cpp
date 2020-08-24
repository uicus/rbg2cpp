#include"modifiers_counter.hpp"
#include"sum.hpp"
#include"concatenation.hpp"
#include"star_move.hpp"
#include"offs.hpp"
#include"assignments.hpp"
#include"switch.hpp"

std::vector<uint> modifiers_counter::get_result(void){
    return std::move(modifiers_count_to_actions_count);
}

std::vector<uint> modifiers_counter::get_switches_indices(void){
    return std::move(switches_indices);
}

void modifiers_counter::dispatch(const rbg_parser::sum& m){
    for(const auto& el: m.get_content())
        el->accept(*this);
}

void modifiers_counter::dispatch(const rbg_parser::concatenation& m){
    for(const auto& el: m.get_content())
        el->accept(*this);
}

void modifiers_counter::dispatch(const rbg_parser::star_move& m){
    m.get_content()->accept(*this);
}

void modifiers_counter::dispatch(const rbg_parser::shift&){
}

void modifiers_counter::dispatch(const rbg_parser::noop&){
}

void modifiers_counter::dispatch(const rbg_parser::ons&){
}

void modifiers_counter::dispatch(const rbg_parser::off& m){
    modifiers_count_to_actions_count.emplace_back(m.index_in_expression());
}

void modifiers_counter::dispatch(const rbg_parser::assignment& m){
    modifiers_count_to_actions_count.emplace_back(m.index_in_expression());
}

void modifiers_counter::dispatch(const rbg_parser::player_switch& m){
    modifiers_count_to_actions_count.emplace_back(m.index_in_expression());
    switches_indices.emplace_back(m.index_in_expression());
}

void modifiers_counter::dispatch(const rbg_parser::keeper_switch& m){
    modifiers_count_to_actions_count.emplace_back(m.index_in_expression());
    switches_indices.emplace_back(m.index_in_expression());
}

void modifiers_counter::dispatch(const rbg_parser::move_check&){
}

void modifiers_counter::dispatch(const rbg_parser::arithmetic_comparison&){
}
