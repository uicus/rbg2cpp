#include"monotonocity_determiner.hpp"
#include"offs.hpp"

void monotonicity_determiner::dispatch(const rbg_parser::shift& m){
}

void monotonicity_determiner::dispatch(const rbg_parser::ons& m){
}

void monotonicity_determiner::dispatch(const rbg_parser::off& m){
    all_used_offs.emplace(m.get_piece());
}

void monotonicity_determiner::dispatch(const rbg_parser::assignment& m){
}

void monotonicity_determiner::dispatch(const rbg_parser::player_switch& m){
}

void monotonicity_determiner::dispatch(const rbg_parser::keeper_switch& m){
}

void monotonicity_determiner::dispatch(const rbg_parser::move_check& m){
}

void monotonicity_determiner::dispatch(const rbg_parser::arithmetic_comparison& m){
}

std::set<rbg_parser::token> monotonicity_determiner::get_ons_from_monotonics(void)const{
    return all_ons_in_monotonics;
}

std::set<rbg_parser::token> monotonicity_determiner::get_all_offs(void)const{
    return all_used_offs;
}

void monotonicity_determiner::notify_about_monotonic_end(void){
    all_ons_in_monotonics.merge(ons_in_current_monotonic);
    ons_in_current_monotonic.clear();
}
