#include"cache_checks_container.hpp"
#include"automaton.hpp"
#include"rules_board_automaton.hpp"

cache_checks_container::cache_checks_container(const automaton& main_automaton,
                                               const std::vector<automaton>& pattern_automata,
                                               const std::vector<shift_table>& shift_tables,
                                               const std::vector<precomputed_pattern>& precomputed_patterns,
                                               const std::vector<std::vector<uint>>& board_structure,
                                               const std::map<rbg_parser::token, uint>& edges_to_id){
    auto main_rba = main_automaton.generate_rules_board_automaton(shift_tables,
                                                                  precomputed_patterns,
                                                                  board_structure,
                                                                  edges_to_id);
    main_automation_checks = main_rba.get_cache_checks_need();
    for(const auto& el: pattern_automata){
        auto pattern_rba = el.generate_rules_board_automaton(shift_tables,
                                                             precomputed_patterns,
                                                             board_structure,
                                                             edges_to_id);
        pattern_automata_checks.emplace_back(pattern_rba.get_cache_checks_need());
    }
}

bool cache_checks_container::should_cache_be_checked(uint state)const{
    return main_automation_checks[state];
}

bool cache_checks_container::should_cache_be_checked_in_pattern(uint state, uint pattern)const{
    return pattern_automata_checks[pattern][state];
}
