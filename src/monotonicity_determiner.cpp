#include"monotonicity_determiner.hpp"
#include"offs.hpp"
#include"ons.hpp"
#include"shift_table.hpp"
#include<algorithm>

void monotonicity_determiner::handle_non_monotonic_action(void){
    current_state = beginning;
    if(current_state == only_finishing_switch_acceptable or current_state == after_first_finishing_switch or current_state == ruined)
        current_state = ruined;
}

void monotonicity_determiner::dispatch(const rbg_parser::shift&){
    handle_non_monotonic_action();
}

void monotonicity_determiner::dispatch(const rbg_parser::ons& m){
    if(current_state == after_shift_table){
        if(current_move.pieces_choice.has_value()){
            std::set<rbg_parser::token> new_set;
            std::set_intersection(current_move.pieces_choice->begin(), current_move.pieces_choice->end(),
                                  m.get_legal_ons().begin(), m.get_legal_ons().end(),
                                  std::inserter(new_set, new_set.begin()));
            std::swap(new_set, *current_move.pieces_choice);
        }
        else
            current_move.pieces_choice = m.get_legal_ons();
    }
    else
        handle_non_monotonic_action();
}

void monotonicity_determiner::dispatch(const rbg_parser::off& m){
    all_used_offs.emplace(m.get_piece());
    handle_non_monotonic_action();
}

void monotonicity_determiner::dispatch(const rbg_parser::assignment&){
    handle_non_monotonic_action();
}

void monotonicity_determiner::dispatch(const rbg_parser::player_switch&){
    if(current_state == after_shift_table or current_state == only_finishing_switch_acceptable or current_state == after_first_finishing_switch){
        current_move.end_states.emplace_back(automaton_state);
        current_state = after_first_finishing_switch;
    }
    else{
        current_state = after_initial_switch;
        current_move = {automaton_state, nullptr, {}, {}};
    }
}

void monotonicity_determiner::dispatch(const rbg_parser::keeper_switch&){
    if(current_state == after_shift_table or current_state == only_finishing_switch_acceptable or current_state == after_first_finishing_switch){
        current_move.end_states.emplace_back(automaton_state);
        current_state = after_first_finishing_switch;
    }
    else
        handle_non_monotonic_action();
}

void monotonicity_determiner::dispatch(const rbg_parser::move_check&){
    handle_non_monotonic_action();
}

void monotonicity_determiner::dispatch(const rbg_parser::arithmetic_comparison&){
    handle_non_monotonic_action();
}

void monotonicity_determiner::dispatch_shift_table(const shift_table& table){
    if(current_state == after_initial_switch and table.are_all_the_same()){
        current_state = after_shift_table;
        current_move.cell_choice = &table;
    }
    else
        handle_non_monotonic_action();
}

void monotonicity_determiner::dispatch_other_action(void){
    handle_non_monotonic_action();
}

std::set<rbg_parser::token> monotonicity_determiner::get_all_offs(void)const{
    return all_used_offs;
}

void monotonicity_determiner::notify_about_last_alternative(void){
    if(current_state == after_first_finishing_switch){
        monotonics.emplace_back(current_move);
        current_state = beginning;
    }
}

void monotonicity_determiner::notify_about_alternative_start(void){
    if(current_state == after_shift_table)
        current_state = only_finishing_switch_acceptable;
}

void monotonicity_determiner::notify_about_automaton_state(uint state){
    automaton_state = state;
}

bool monotonicity_determiner::is_monotonic_ruined_by_off(const monotonic_move& m)const{
    if(not m.pieces_choice.has_value())
        return false;
    else{
        for(const auto& el: *m.pieces_choice)
            if(all_used_offs.count(el) > 0)
                return true;
        return false;
    }
}

std::vector<monotonic_move> monotonicity_determiner::get_final_result(void)const{
    std::vector<monotonic_move> final_result;
    std::copy_if(monotonics.begin(), monotonics.end(), std::back_inserter(final_result),
        [this](const auto& el){return not is_monotonic_ruined_by_off(el);});
    return final_result;
}
