#include"rules_board_automaton.hpp"
#include"edge.hpp"

rules_board_automaton::rules_board_automaton(uint num_of_states, uint num_of_cells)
  : potential_starter(num_of_states)
  , check_needed(num_of_states)
  , visited(num_of_states, num_of_cells)
  , states(num_of_states){
    for(auto& el: states)
        el.resize(num_of_cells);
}

void rules_board_automaton::report_edge_from_original_automaton(
    const edge& e,
    uint starting_state,
    const std::vector<shift_table>& shift_tables,
    const std::vector<precomputed_pattern>& precomputed_patterns,
    const std::vector<std::vector<uint>>& board_structure,
    const std::map<rbg_parser::token, uint>& edges_to_id){
    auto& starting_states = states[starting_state];
    const auto target_state = e.get_endpoint();
    for(uint i=0;i<starting_states.size();++i)
    {
        auto [modifier_encountered, next_cells] = e.build_next_cells_edges(i, shift_tables, precomputed_patterns, board_structure, edges_to_id);
        for(auto el: next_cells)
            starting_states[i].edges.emplace_back(edge_info{target_state, el, modifier_encountered});
        if(modifier_encountered)
            potential_starter[target_state] = true;
    }
}

void rules_board_automaton::report_start_state(uint starting_state){
    potential_starter[starting_state] = true;
}

void rules_board_automaton::run_dfs_from(uint state, uint cell){
    visited.reset();
    std::vector<std::pair<uint,uint>> dfs_stack = {{state,cell}};
    while(not dfs_stack.empty()){
        auto [next_state, next_cell] = dfs_stack.back();
        dfs_stack.pop_back();
        if(visited.is_set(next_state, next_cell))
            check_needed[next_state] = true;
        else{
            visited.set(next_state, next_cell);
            for(const auto& el: states[next_state][next_cell].edges)
                if(not el.crosses_modifier)
                    dfs_stack.emplace_back(el.target_state, el.target_cell);
        }
    }
}

std::vector<bool> rules_board_automaton::get_cache_checks_need(void){
    std::fill(check_needed.begin(), check_needed.end(), false);
    for(uint i=0;i<states.size();++i)
        if(potential_starter[i])
            for(uint j=0;j<states[i].size();++j)
                run_dfs_from(i, j);
    return check_needed;
}
