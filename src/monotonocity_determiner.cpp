#include"monotonocity_determiner.hpp"
#include"offs.hpp"
#include"ons.hpp"
#include"shift_table.hpp"

void monotonicity_determiner::handle_non_monotonic_action(void){
    current_state = beginning;
    if(current_state == only_finishing_switch_acceptable or current_state == after_first_finishing_switch or current_state == ruined)
        current_state = ruined;
}

void monotonicity_determiner::dispatch(const rbg_parser::shift&){
    handle_non_monotonic_action();
}

void monotonicity_determiner::dispatch(const rbg_parser::ons& m){
    if(current_state == after_shift_table)
        current_move.pieces_choice.insert(m.get_legal_ons().begin(), m.get_legal_ons().end());
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
    if(current_state == after_shift_table or current_state == only_finishing_switch_acceptable)
        current_state = after_first_finishing_switch;
    else{
        current_state = after_initial_switch;
        current_move = {automaton_state, nullptr, {}};
    }
}

void monotonicity_determiner::dispatch(const rbg_parser::keeper_switch&){
    if(current_state == after_shift_table or current_state == only_finishing_switch_acceptable)
        current_state = after_first_finishing_switch;
    else
        current_state = after_initial_switch;
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

std::set<rbg_parser::token> monotonicity_determiner::get_all_offs(void)const{
    return all_used_offs;
}

void monotonicity_determiner::notify_about_last_alternative(void){
    if(current_state == after_first_finishing_switch)
        monotonics.emplace_back(current_move);
}

void monotonicity_determiner::notify_about_alternative_start(void){
    if(current_state == after_shift_table)
        current_state = only_finishing_switch_acceptable;
}

void monotonicity_determiner::notify_about_automaton_state(uint state){
    automaton_state = state;
}
