#ifndef CACHE_CHECKS_CONTAINER
#define CACHE_CHECKS_CONTAINER

#include<vector>
#include<map>
#include<optional>

class automaton;
class shift_table;
class precomputed_pattern;

namespace rbg_parser{
    class token;
}

class cache_checks_container{
        struct cache_info{
            uint real_size;
            std::vector<std::optional<uint>> states_to_cache_correspondence;
        };
        cache_info main_automation_checks = {};
        std::vector<cache_info> pattern_automata_checks = {};
        cache_info transform_mask_into_cache_info(const std::vector<bool>& checks_bitmask)const;
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
        bool is_main_cache_needed(void)const;
        bool should_cache_be_checked_in_pattern(uint state, uint pattern)const;
        bool is_pattern_cache_needed(uint pattern)const;
        bool is_any_cache_needed(void)const;
};

#endif
