#include"rules_board_automaton.hpp"
#include"edge.hpp"

rules_board_automaton::rules_board_automaton(uint num_of_states, uint num_of_cells)
  : potential_starter(num_of_states)
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
        for(auto el: next_cells){
            starting_states[i].edges.emplace_back(edge_info{target_state, el, modifier_encountered});
        }
        if(modifier_encountered)
            potential_starter[target_state] = true;
    }
}
