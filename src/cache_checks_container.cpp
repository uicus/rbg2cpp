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
    main_automation_checks = transform_mask_into_cache_info(main_rba.get_cache_checks_need());
    for(const auto& el: pattern_automata){
        auto pattern_rba = el.generate_rules_board_automaton(shift_tables,
                                                             precomputed_patterns,
                                                             board_structure,
                                                             edges_to_id);
        pattern_automata_checks.emplace_back(transform_mask_into_cache_info(pattern_rba.get_cache_checks_need()));
    }
}

cache_checks_container::cache_info cache_checks_container::transform_mask_into_cache_info(const std::vector<bool>& checks_bitmask)const{
    cache_info result{0,std::vector<std::optional<uint>>(checks_bitmask.size(),std::nullopt)};
    for(uint i=0;i<checks_bitmask.size();++i)
        if(checks_bitmask[i])
            result.states_to_cache_correspondence[i] = result.real_size++;
    return result;
}

bool cache_checks_container::should_cache_be_checked(uint state)const{
    return main_automation_checks.states_to_cache_correspondence[state].has_value();
}

bool cache_checks_container::should_cache_be_checked_in_pattern(uint state, uint pattern)const{
    return pattern_automata_checks[pattern].states_to_cache_correspondence[state].has_value();
}

bool cache_checks_container::is_main_cache_needed(void)const{
    return main_automation_checks.real_size > 0;
}

bool cache_checks_container::is_pattern_cache_needed(uint pattern)const{
    return pattern_automata_checks[pattern].real_size > 0;
}

bool cache_checks_container::is_any_cache_needed(void)const{
    if(is_main_cache_needed())
        return true;
    for(uint i=0;i<pattern_automata_checks.size();++i)
        if(is_pattern_cache_needed(i))
            return true;
    return false;
}
