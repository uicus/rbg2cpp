#ifndef TRANSITION_DATA
#define TRANSITION_DATA

#include"types.hpp"
#include<map>
#include<vector>
#include<string>

namespace rbg_parser{
    class token;
    class declarations;
}

class shift_table;
class precomputed_pattern;

enum kind_of_transition{
    all_getter,
    any_getter,
    inside_pattern,
};

struct static_transition_data{
    const std::map<rbg_parser::token, uint>& pieces_to_id;
    const std::map<rbg_parser::token, uint>& edges_to_id;
    const std::map<rbg_parser::token, uint>& variables_to_id;
    const rbg_parser::declarations& decl;
    const std::vector<shift_table>& shift_tables;
    const std::vector<precomputed_pattern>& precomputed_patterns;
    std::string return_type;
    std::string name_prefix;
    std::string success_finish;
    std::string failure_finish;
    std::string cache_level_getter;
    std::string cache_level_pusher;
    std::string cache_level_reverter;
    std::string cache_setter;
    std::string cache_set_getter;
    kind_of_transition kind;
    static_transition_data(
        const std::map<rbg_parser::token, uint>& pieces_to_id,
        const std::map<rbg_parser::token, uint>& edges_to_id,
        const std::map<rbg_parser::token, uint>& variables_to_id,
        const rbg_parser::declarations& decl,
        const std::vector<shift_table>& shift_tables,
        const std::vector<precomputed_pattern>& precomputed_patterns,
        const std::string& name_prefix,
        const std::string& cache_level_getter,
        const std::string& cache_level_pusher,
        const std::string& cache_level_reverter,
        const std::string& cache_setter,
        const std::string& cache_set_getter,
        kind_of_transition kind);
};



#endif
