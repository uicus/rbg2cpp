#include"transition_data.hpp"
#include"token.hpp"
#include"declarations.hpp"
#include"shift_table.hpp"
#include"precomputed_pattern.hpp"

static_transition_data::static_transition_data(
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
    kind_of_transition kind)
      : pieces_to_id(pieces_to_id),
        edges_to_id(edges_to_id),
        variables_to_id(variables_to_id),
        decl(decl),
        shift_tables(shift_tables),
        precomputed_patterns(precomputed_patterns),
        return_type(),
        name_prefix(name_prefix),
        success_finish(),
        failure_finish(),
        cache_level_getter(cache_level_getter),
        cache_level_pusher(cache_level_pusher),
        cache_level_reverter(cache_level_reverter),
        cache_setter(cache_setter),
        cache_set_getter(cache_set_getter),
        kind(kind){
    switch(kind){
        case all_getter:
            return_type = "void";
            success_finish = failure_finish = "return;";
            break;
        case any_getter:
        case inside_pattern:
            return_type = "bool";
            success_finish = "return true;";
            failure_finish = "return false;";
            break;
    }
}


