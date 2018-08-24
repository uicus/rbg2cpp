#include"state.hpp"
#include"compiler_options.hpp"
#include"cpp_container.hpp"
#include<cassert>

void state::inform_about_being_appended(uint shift_value){
    for(auto& el: next_states)
        el.shift(shift_value);
}

void state::inform_about_state_deletion(uint deleted_index){
    for(auto& el: next_states)
        el.inform_abut_state_deletion(deleted_index);
}

void state::absorb(state&& rhs){
    assert(next_states.empty());
    if(not rhs.next_states.empty()){
        next_states = std::move(rhs.next_states);
        outgoing_edges_needed |= rhs.outgoing_edges_needed;
        doubly_reachable |= rhs.doubly_reachable;
        rhs.next_states.clear();
    }
}

void state::connect_with_state(uint index_in_local_register, const std::vector<label>& label_list){
    next_states.push_back(edge(index_in_local_register, label_list));
}

void state::print_transition_functions(
    uint from_state,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::vector<state>& local_register,
    const compiler_options& opts)const{
    if(next_states.size()>1 or outgoing_edges_needed)
        for(const auto& el: next_states)
            el.print_transition_function(from_state, output, pieces_to_id, edges_to_id, variables_to_id, decl, local_register,opts);
}

void state::print_transition_functions_inside_pattern(
    uint from_state,
    uint pattern_index,
    cpp_container& output,
    const std::map<rbg_parser::token, uint>& pieces_to_id,
    const std::map<rbg_parser::token, uint>& edges_to_id,
    const std::map<rbg_parser::token, uint>& variables_to_id,
    const rbg_parser::declarations& decl,
    const std::vector<state>& local_register,
    const compiler_options& opts)const{
    if(next_states.size()>1 or outgoing_edges_needed)
        for(const auto& el: next_states)
            el.print_transition_function_inside_pattern(from_state, pattern_index, output, pieces_to_id, edges_to_id, variables_to_id, decl, local_register,opts);
}

void state::print_outgoing_transitions(uint from_state, cpp_container& output, const std::string& functions_prefix)const{
    std::string resulting_line = "{";
    if(next_states.size()>1 or outgoing_edges_needed)
        for(uint i=0;i<next_states.size();++i){
            resulting_line += "&next_states_iterator::"+functions_prefix+"_"+std::to_string(from_state)+"_"+std::to_string(next_states[i].get_endpoint());
            if(i+1<next_states.size())
                resulting_line += ',';
        }
    resulting_line += "},";
    output.add_source_line(resulting_line);
}

void state::notify_endpoints_about_being_reachable(std::vector<uint>& reachability)const{
    for(const auto& el: next_states)
        ++reachability[el.get_endpoint()];
}

void state::mark_as_doubly_reachable(void){
    doubly_reachable = true;
}

bool state::can_be_checked_for_visit(void)const{
    return doubly_reachable;
}

void state::mark_explicitly_as_transition_start(void){
    outgoing_edges_needed = true;
}

const edge& state::get_only_exit(void)const{
    assert(is_no_choicer());
    return next_states[0];
}

bool state::is_dead_end(void)const{
    return next_states.empty();
}

bool state::is_no_choicer(void)const{
    return next_states.size() == 1;
}

void state::push_next_states_to_shift_tables_dfs_stack(
    uint current_cell,
    const rbg_parser::graph& board,
    std::vector<std::pair<uint,uint>>& dfs_stack,
    const std::vector<precomputed_pattern>& pps)const{
    for(const auto& el: next_states){
        auto cell = el.get_next_cell(current_cell,board,pps);
        if(cell>=0)
            dfs_stack.emplace_back(el.get_endpoint(), uint(cell));
    }
}
