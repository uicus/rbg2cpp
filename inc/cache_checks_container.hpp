#ifndef CACHE_CHECKS_CONTAINER
#define CACHE_CHECKS_CONTAINER

#include<vector>
#include<map>

class automaton;
class shift_table;
class precomputed_pattern;

namespace rbg_parser{
    class token;
}

class cache_checks_container{
        std::vector<bool> main_automation_checks = {};
        std::vector<std::vector<bool>> pattern_automata_checks = {};
    public:
        cache_checks_container(void)=default;
        cache_checks_container(const cache_checks_container&)=delete;
        cache_checks_container(cache_checks_container&&)=default;
        cache_checks_container& operator=(const cache_checks_container&)=delete;
        cache_checks_container& operator=(cache_checks_container&&)=default;
        ~cache_checks_container(void)=default;
        cache_checks_container(const automaton& main_automaton,
                               const std::vector<automaton>& pattern_automata,
                               const std::vector<shift_table>& shift_tables,
                               const std::vector<precomputed_pattern>& precomputed_patterns,
                               const std::vector<std::vector<uint>>& board_structure,
                               const std::map<rbg_parser::token, uint>& edges_to_id);
        bool should_cache_be_checked(uint state)const;
        bool should_cache_be_checked_in_pattern(uint state, uint pattern)const;
};

#endif
