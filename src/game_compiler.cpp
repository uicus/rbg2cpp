#include"game_compiler.hpp"
#include"compiler_options.hpp"
#include"automaton_builder.hpp"
#include"parsed_game.hpp"
#include"types.hpp"
#include<algorithm>

game_compiler::game_compiler(const rbg_parser::parsed_game& input, const compiler_options& opts):
output(opts.output_file()),
opts(opts),
name(opts.output_file()),
pieces_to_id(),
game_automaton(),
pattern_automata(),
patterns_offsets(),
patterns_size(0),
input(input){
}

void game_compiler::generate_board_cells_decoder(void){
    output.add_header_include("string");
    const auto& g = input.get_board();
    output.add_source_line("static const std::string cells_names["+std::to_string(g.get_size())+"] = {");
    for(uint i=0;i<g.get_size();++i)
        output.add_source_line("\""+g.get_vertex(i).to_string()+"\",");
    output.add_source_line("};");
    output.add_source_line("");
    output.add_header_line("std::string board_cell_to_string(int cell_id);");
    output.add_source_line("std::string board_cell_to_string(int cell_id){");
    output.add_source_line("return cells_names[cell_id];");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_pieces_decoder(void){
    output.add_header_include("string");
    const auto& p = input.get_declarations().get_legal_pieces();
    output.add_source_line("static const std::string pieces_names["+std::to_string(p.size())+"] = {");
    uint current_id = 0;
    for(const auto& el: p){
        output.add_source_line("\""+el.to_string()+"\",");
        pieces_to_id.insert(std::make_pair(el, current_id++));
    }
    output.add_source_line("};");
    output.add_source_line("");
    output.add_header_line("std::string piece_to_string(int piece_id);");
    output.add_source_line("std::string piece_to_string(int piece_id){");
    output.add_source_line("return pieces_names[piece_id];");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_variables_decoder(void){
    output.add_header_include("string");
    const auto& v = input.get_declarations().get_legal_variables();
    const auto& p = input.get_declarations().get_legal_players();
    output.add_source_line("static const std::string variables_names["+std::to_string(p.size()+v.size())+"] = {");
    uint current_id = 0;
    for(const auto& el: p){
        output.add_source_line("\""+el.first.to_string()+"\",");
        variables_to_id.insert(std::make_pair(el.first, current_id++));
    }
    for(const auto& el: v){
        output.add_source_line("\""+el.first.to_string()+"\",");
        variables_to_id.insert(std::make_pair(el.first, current_id++));
    }
    output.add_source_line("};");
    output.add_source_line("");
    output.add_header_line("std::string variable_to_string(int variable_id);");
    output.add_source_line("std::string variable_to_string(int variable_id){");
    output.add_source_line("return variables_names[variable_id];");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::fill_edges_map(void){
    const auto& e = input.get_declarations().get_legal_edges();
    uint current_id = 0;
    for(const auto& el: e)
        edges_to_id.insert(std::make_pair(el, current_id++));
}

std::string game_compiler::numbers_to_array(const std::vector<int>& numbers)const{
    std::string result = "{";
    for(uint i=0;i<numbers.size()-1;++i)
        result += std::to_string(numbers[i])+",";
    if(not numbers.empty())
        result += std::to_string(numbers.back());
    result += "}";
    return result;
}

void game_compiler::generate_board_structure(void){
    const auto& g = input.get_board();
    output.add_source_line("static const int cell_neighbors["+std::to_string(g.get_size()+1)+"]["+std::to_string(edges_to_id.size())+"] = {");
    output.add_source_line(numbers_to_array(std::vector<int>(edges_to_id.size(),0))+",");
    for(uint i=0;i<g.get_size();++i){
        const auto& outgoing_edges = g.get_outgoing_edges(i);
        std::vector<int> neighbors(edges_to_id.size(),0);
        for(const auto& el: outgoing_edges)
            neighbors[edges_to_id[el.first]] = int(el.second+1);
        output.add_source_line(numbers_to_array(neighbors)+",");
    }
    output.add_source_line("};");
    output.add_source_line("");
    output.add_header_line("int get_neighbor(int cell_id, int edge_id);");
    output.add_source_line("int get_neighbor(int cell_id, int edge_id){");
    output.add_source_line("return cell_neighbors[cell_id][edge_id];");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_variables_bounds(void){
    const auto& v = input.get_declarations().get_legal_variables();
    const auto& p = input.get_declarations().get_legal_players();
    output.add_source_line("static const int bounds["+std::to_string(variables_to_id.size())+"] = {");
    for(const auto& el: p)
        output.add_source_line(std::to_string(el.second)+",");
    for(const auto& el: v)
        output.add_source_line(std::to_string(el.second)+",");
    output.add_source_line("};");
    output.add_source_line("");
    output.add_header_line("int get_bound(int variable_id);");
    output.add_source_line("int get_bound(int variable_id){");
    output.add_source_line("return bounds[variable_id];");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_initial_pieces(void){
    const auto& g = input.get_board();
    output.add_header_line("int pieces["+std::to_string(g.get_size()+1)+"] = {");
    output.add_header_line("-1,");
    std::vector<int> pieces_count(pieces_to_id.size(),0);
    for(uint i=0;i<g.get_size();++i){
        output.add_header_line(std::to_string(pieces_to_id[g.get_starting_piece(i)])+",");
        ++pieces_count[pieces_to_id[g.get_starting_piece(i)]];
    }
    output.add_header_line("};");
    std::string pieces_count_generated_array = "int pieces_count["+std::to_string(pieces_to_id.size())+"] = {";
    for(uint i=0;i<pieces_count.size();++i)
        pieces_count_generated_array += std::to_string(pieces_count[i]) + (i==pieces_count.size()-1?"":",");
    pieces_count_generated_array += "};";
    output.add_header_line(pieces_count_generated_array);
}

void game_compiler::generate_initial_variables(void){
    output.add_header_line("int variables["+std::to_string(variables_to_id.size())+"] = {};");
}

void game_compiler::generate_state_getters(void){
    output.add_header_line("int get_current_cell(void)const;");
    output.add_source_line("int game_state::get_current_cell(void)const{");
    output.add_source_line("return current_cell-1;");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("int get_piece(int cell_id)const;");
    output.add_source_line("int game_state::get_piece(int cell_id)const{");
    output.add_source_line("return pieces[cell_id+1];");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("int get_variable_value(int variable_id)const;");
    output.add_source_line("int game_state::get_variable_value(int variable_id)const{");
    output.add_source_line("return variables[variable_id];");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("int get_current_player(void)const;");
    output.add_source_line("int game_state::get_current_player(void)const{");
    output.add_source_line("return current_player;");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("int get_player_score(int player_id)const;");
    output.add_source_line("int game_state::get_player_score(int player_id)const{");
    output.add_source_line("return variables[player_id-1];");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_game_state_class(void){
    output.add_header_line("class game_state{");
    output.add_header_line("public:");
    generate_state_getters();
    output.add_header_line("void apply_move(const move& m);");
    output.add_source_line("void game_state::apply_move(const move& m){");
    output.add_source_line("current_cell = m.next_cell;");
    output.add_source_line("current_player = m.next_player;");
    output.add_source_line("current_state = m.next_state;");
    output.add_source_line("apply_board_changes_list(m.board_list);");
    output.add_source_line("apply_variables_changes_list(m.variables_list);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("friend class next_states_iterator;");
    output.add_header_line("private:");
    output.add_header_line("void apply_board_changes_list(const std::shared_ptr<board_appliers>& list);");
    output.add_source_line("void game_state::apply_board_changes_list(const std::shared_ptr<board_appliers>& list){");
    output.add_source_line("if(list){");
    output.add_source_line("apply_board_changes_list(list->tail);");
    output.add_source_line("--pieces_count[pieces[list->head.cell]];");
    output.add_source_line("++pieces_count[list->head.previous_piece];");
    output.add_source_line("pieces[list->head.cell] = list->head.previous_piece;");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void apply_variables_changes_list(const std::shared_ptr<variables_appliers>& list);");
    output.add_source_line("void game_state::apply_variables_changes_list(const std::shared_ptr<variables_appliers>& list){");
    output.add_source_line("if(list){");
    output.add_source_line("apply_variables_changes_list(list->tail);");
    output.add_source_line("variables[list->head.variable] = list->head.previous_value;");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("int current_cell = 1;");
    output.add_header_line("int current_player = 0;");
    output.add_header_line("int current_state = "+std::to_string(game_automaton.get_start_state())+";");
    generate_initial_pieces();
    generate_initial_variables();
    output.add_header_line("};");
    output.add_header_line("");
}

void game_compiler::generate_game_parameters(void){
    const auto& g = input.get_board();
    const auto& pl = input.get_declarations().get_legal_players();
    const auto& pi = input.get_declarations().get_legal_pieces();
    const auto& v = input.get_declarations().get_legal_variables();
    const auto& e = input.get_declarations().get_legal_edges();
    output.add_header_line("constexpr int BOARD_SIZE = "+std::to_string(g.get_size())+";");
    output.add_header_line("constexpr int NUMBER_OF_PLAYERS = "+std::to_string(pl.size()+1)+";");
    output.add_header_line("constexpr int NUMBER_OF_PIECES = "+std::to_string(pi.size())+";");
    output.add_header_line("constexpr int NUMBER_OF_VARIABLES = "+std::to_string(pl.size()+v.size())+";");
    output.add_header_line("constexpr int BOARD_DEGREE = "+std::to_string(e.size())+";");
    output.add_header_line("");
}

void game_compiler::build_game_automaton(void){
    automaton_builder b(pattern_automata);
    input.get_moves()->accept(b);
    game_automaton = b.get_final_result();
    game_automaton.mark_start_as_outgoing_usable();
    game_automaton.mark_states_as_double_reachable();
    for(auto& el: pattern_automata){
        el.mark_start_as_outgoing_usable();
        el.mark_states_as_double_reachable();
        patterns_offsets.push_back(patterns_size);
        patterns_size += el.get_size();
    }
}

void game_compiler::generate_iterator_helper_structures(void){
    output.add_header_line("struct backtrace_information{");
    output.add_header_line("unsigned int current_branch;");
    if(opts.enabled_shift_tables())
        output.add_header_line("unsigned int current_shift_table_branch;");
    output.add_header_line("int state_checkpoint;");
    output.add_header_line("int current_cell_checkpoint;");
    output.add_header_line("unsigned int modifiers_depth_checkpoint;");
    output.add_header_line("unsigned int board_checkpoint;");
    output.add_header_line("unsigned int variables_checkpoint;");
    output.add_header_line("backtrace_information(void)=default;");
    output.add_header_line("backtrace_information(unsigned int current_branch,"+(opts.enabled_shift_tables()?"unsigned int current_shift_table_branch,":std::string())+"int state_checkpoint,int current_cell_checkpoint,unsigned int modifiers_depth_checkpoint,unsigned int board_checkpoint,unsigned int variables_checkpoint)");
    output.add_header_line(": current_branch(current_branch),");
    if(opts.enabled_shift_tables())
        output.add_header_line("  current_shift_table_branch(current_shift_table_branch),");
    output.add_header_line("  state_checkpoint(state_checkpoint),");
    output.add_header_line("  current_cell_checkpoint(current_cell_checkpoint),");
    output.add_header_line("  modifiers_depth_checkpoint(modifiers_depth_checkpoint),");
    output.add_header_line("  board_checkpoint(board_checkpoint),");
    output.add_header_line("  variables_checkpoint(variables_checkpoint){");
    output.add_header_line("}");
    output.add_header_line("};");
    output.add_header_line("struct board_changes_information{");
    output.add_header_line("int cell;");
    output.add_header_line("int previous_piece;");
    output.add_header_line("board_changes_information(void)=default;");
    output.add_header_line("board_changes_information(int cell,int previous_piece)");
    output.add_header_line(": cell(cell),");
    output.add_header_line("  previous_piece(previous_piece){");
    output.add_header_line("}");
    output.add_header_line("};");
    output.add_header_line("struct variable_changes_information{");
    output.add_header_line("int variable;");
    output.add_header_line("int previous_value;");
    output.add_header_line("variable_changes_information(void)=default;");
    output.add_header_line("variable_changes_information(int variable,int previous_value)");
    output.add_header_line(": variable(variable),");
    output.add_header_line("  previous_value(previous_value){");
    output.add_header_line("}");
    output.add_header_line("};");
    output.add_header_line("");
}

void game_compiler::generate_iterator_revert_methods(void){
    output.add_header_line("void revert_board_changes(unsigned int to_point);");
    output.add_source_line("void next_states_iterator::revert_board_changes(unsigned int to_point){");
    output.add_source_line("for(unsigned int i=board_change_points.size();i>to_point;--i){");
    output.add_source_line("--state_to_change.pieces_count[state_to_change.pieces[board_change_points[i-1].cell]];");
    output.add_source_line("++state_to_change.pieces_count[board_change_points[i-1].previous_piece];");
    output.add_source_line("state_to_change.pieces[board_change_points[i-1].cell] = board_change_points[i-1].previous_piece;");
    output.add_source_line("board_list = board_list->tail;");
    output.add_source_line("}");
    output.add_source_line("board_change_points.resize(to_point);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert_variables_changes(unsigned int to_point);");
    output.add_source_line("void next_states_iterator::revert_variables_changes(unsigned int to_point){");
    output.add_source_line("for(unsigned int i=variables_change_points.size();i>to_point;--i){");
    output.add_source_line("state_to_change.variables[variables_change_points[i-1].variable] = variables_change_points[i-1].previous_value;");
    output.add_source_line("variables_list = variables_list->tail;");
    output.add_source_line("}");
    output.add_source_line("variables_change_points.resize(to_point);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert_board_changes_after_pattern(unsigned int to_point);");
    output.add_source_line("void next_states_iterator::revert_board_changes_after_pattern(unsigned int to_point){");
    output.add_source_line("for(unsigned int i=board_change_points.size();i>to_point;--i){");
    output.add_source_line("--state_to_change.pieces_count[state_to_change.pieces[board_change_points[i-1].cell]];");
    output.add_source_line("++state_to_change.pieces_count[board_change_points[i-1].previous_piece];");
    output.add_source_line("state_to_change.pieces[board_change_points[i-1].cell] = board_change_points[i-1].previous_piece;");
    output.add_source_line("}");
    output.add_source_line("board_change_points.resize(to_point);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert_variables_changes_after_pattern(unsigned int to_point);");
    output.add_source_line("void next_states_iterator::revert_variables_changes_after_pattern(unsigned int to_point){");
    output.add_source_line("for(unsigned int i=variables_change_points.size();i>to_point;--i){");
    output.add_source_line("state_to_change.variables[variables_change_points[i-1].variable] = variables_change_points[i-1].previous_value;");
    output.add_source_line("}");
    output.add_source_line("variables_change_points.resize(to_point);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert_to_point(const backtrace_information& point_to_revert);");
    output.add_source_line("void next_states_iterator::revert_to_point(const backtrace_information& point_to_revert){");
    output.add_source_line("state_to_change.current_cell = point_to_revert.current_cell_checkpoint;");
    output.add_source_line("cache.revert_to_level(point_to_revert.modifiers_depth_checkpoint);");
    output.add_source_line("revert_board_changes(point_to_revert.board_checkpoint);");
    output.add_source_line("revert_variables_changes(point_to_revert.variables_checkpoint);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert_to_point_after_pattern(const backtrace_information& point_to_revert, void(resettable_bitarray_stack::*cache_reverter)(unsigned int));");
    output.add_source_line("void next_states_iterator::revert_to_point_after_pattern(const backtrace_information& point_to_revert, void(resettable_bitarray_stack::*cache_reverter)(unsigned int)){");
    output.add_source_line("state_to_change.current_cell = point_to_revert.current_cell_checkpoint;");
    output.add_source_line("(cache.*cache_reverter)(point_to_revert.modifiers_depth_checkpoint);");
    output.add_source_line("revert_board_changes_after_pattern(point_to_revert.board_checkpoint);");
    output.add_source_line("revert_variables_changes_after_pattern(point_to_revert.variables_checkpoint);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert_to_last_choice(void);");
    output.add_source_line("void next_states_iterator::revert_to_last_choice(void){");
    output.add_source_line("for(unsigned int i=decision_points.size();i>0;--i){");
    output.add_source_line("if(decision_points[i-1].current_branch < transitions[decision_points[i-1].state_checkpoint].size()){");
    output.add_source_line("revert_to_point(decision_points[i-1]);");
    output.add_source_line("decision_points.resize(i);");
    output.add_source_line("return;");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("state_to_change.current_state = decision_points[0].state_checkpoint;");
    output.add_source_line("revert_to_point(decision_points[0]);");
    output.add_source_line("decision_points.clear();");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert_to_last_choice_because_failure(std::vector<backtrace_information> next_states_iterator::*decision_stack, const std::vector<transition_function> transition_array[], void(resettable_bitarray_stack::*cache_reverter)(unsigned int));");
    output.add_source_line("void next_states_iterator::revert_to_last_choice_because_failure(std::vector<backtrace_information> next_states_iterator::*decision_stack, const std::vector<transition_function> transition_array[], void(resettable_bitarray_stack::*cache_reverter)(unsigned int)){");
    output.add_source_line("for(unsigned int i=(this->*decision_stack).size();i>0;--i){");
    output.add_source_line("if((this->*decision_stack)[i-1].current_branch < transition_array[(this->*decision_stack)[i-1].state_checkpoint].size()){");
    output.add_source_line("revert_to_point_after_pattern((this->*decision_stack)[i-1], cache_reverter);");
    output.add_source_line("(this->*decision_stack).resize(i);");
    output.add_source_line("return;");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("revert_to_point_after_pattern((this->*decision_stack)[0], cache_reverter);");
    output.add_source_line("(this->*decision_stack).clear();");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_move_getter(void){
    output.add_header_line("move get_move(void)const;");
    output.add_source_line("move next_states_iterator::get_move(void)const{");
    output.add_source_line("move result;");
    output.add_source_line("result.board_list = board_list;");
    output.add_source_line("result.variables_list = variables_list;");
    output.add_source_line("result.next_cell = state_to_change.current_cell;");
    output.add_source_line("result.next_player = state_to_change.current_player;");
    output.add_source_line("result.next_state = state_to_change.current_state;");
    output.add_source_line("return result;");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_resetter(void){
    output.add_header_line("void reset(void);");
    output.add_source_line("void next_states_iterator::reset(void){");
    output.add_source_line("state_to_change.current_player = moving_player;");
    output.add_source_line("state_to_change.current_state = decision_points[0].state_checkpoint;");
    output.add_source_line("revert_to_point(decision_points[0]);");
    output.add_source_line("decision_points.resize(1);");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_main_dfs(void){
    output.add_header_line("bool next(void);");
    output.add_source_line("bool next_states_iterator::next(void){");
    output.add_source_line("ready_to_report = false;");
    output.add_source_line("state_to_change.current_player = moving_player;");
    output.add_source_line("revert_to_last_choice();");
    output.add_source_line("while(not ready_to_report and not decision_points.empty()){");
    output.add_source_line("(this->*transitions[decision_points.back().state_checkpoint][decision_points.back().current_branch++])();");
    output.add_source_line("}");
    output.add_source_line("return not decision_points.empty();");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_dfs_for_pattern(uint pattern_index){
    output.add_header_line("void evaluate"+std::to_string(pattern_index)+"(void);");
    output.add_source_line("void next_states_iterator::evaluate"+std::to_string(pattern_index)+"(void){");
    output.add_source_line("success_to_report"+std::to_string(pattern_index)+" = false;");
    output.add_source_line("cache.pattern_reset"+std::to_string(pattern_index)+"();");
    output.add_source_line("pattern_decision_points"+std::to_string(pattern_index)+".emplace_back(0,"+(opts.enabled_shift_tables()?"0,":std::string())+std::to_string(pattern_automata[pattern_index].get_start_state())+",state_to_change.current_cell,1,board_change_points.size(),variables_change_points.size());");
    output.add_source_line("while(not pattern_decision_points"+std::to_string(pattern_index)+".empty()){");
    output.add_source_line("(this->*pattern_transitions"+std::to_string(pattern_index)+"[pattern_decision_points"+std::to_string(pattern_index)+".back().state_checkpoint][pattern_decision_points"+std::to_string(pattern_index)+".back().current_branch++])();");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_states_iterator(void){
    generate_resettable_bitarray_stack();
    output.add_header_include("vector");
    output.add_header_include("memory");
    output.add_header_line("class next_states_iterator{");
    output.add_header_line("public:");
    output.add_header_line("next_states_iterator(game_state& state_to_change, resettable_bitarray_stack& cache);");
    output.add_source_line("next_states_iterator::next_states_iterator(game_state& state_to_change, resettable_bitarray_stack& cache)");
    output.add_source_line(": state_to_change(state_to_change),");
    output.add_source_line("  cache(cache),");
    output.add_source_line("  moving_player(state_to_change.current_player){");
    output.add_source_line("cache.reset();");
    output.add_source_line("decision_points.emplace_back(0,"+(opts.enabled_shift_tables()?"0,":std::string())+"state_to_change.current_state,state_to_change.current_cell,1,0,0);");
    output.add_source_line("}");
    output.add_source_line("");
    generate_main_dfs();
    generate_move_getter();
    generate_resetter();
    output.add_header_line("private:");
    output.add_header_line("typedef void(next_states_iterator::*transition_function)(void);");
    for(uint i=0;i<pattern_automata.size();++i){
        generate_dfs_for_pattern(i);
        output.add_header_line("");
    }
    generate_iterator_revert_methods();
    game_automaton.print_transition_table(output, "transitions","transition");
    for(uint i=0;i<pattern_automata.size();++i){
        output.add_header_line("");
        pattern_automata[i].print_transition_table(output, "pattern_transitions"+std::to_string(i),"pattern_transition"+std::to_string(i));
    }
    output.add_header_line("");
    game_automaton.print_transition_functions(output,pieces_to_id,edges_to_id,variables_to_id,input.get_declarations(),opts);
    for(uint i=0;i<pattern_automata.size();++i){
        output.add_header_line("");
        pattern_automata[i].print_transition_functions_inside_pattern(i,output,pieces_to_id,edges_to_id,variables_to_id,input.get_declarations(),opts);
    }
    output.add_header_line("");
    output.add_header_line("game_state& state_to_change;");
    output.add_header_line("resettable_bitarray_stack& cache;");
    output.add_header_line("int moving_player;");
    for(uint i=0;i<pattern_automata.size();++i)
        output.add_header_line("bool success_to_report"+std::to_string(i)+" = false;");
    output.add_header_line("bool ready_to_report = false;");
    output.add_header_line("std::vector<backtrace_information> decision_points;");
    for(uint i=0;i<pattern_automata.size();++i)
        output.add_header_line("std::vector<backtrace_information> pattern_decision_points"+std::to_string(i)+";");
    output.add_header_line("std::vector<board_changes_information> board_change_points;");
    output.add_header_line("std::vector<variable_changes_information> variables_change_points;");
    output.add_header_line("std::shared_ptr<board_appliers> board_list;");
    output.add_header_line("std::shared_ptr<variables_appliers> variables_list;");
    output.add_header_line("};");
}

void game_compiler::generate_resettable_bitarray(void){
    output.add_header_include("limits");
    output.add_header_line("template<int states,int cells>");
    output.add_header_line("class resettable_bitarray{");
    output.add_header_line("public:");
    output.add_header_line("inline bool is_set(int state, int cell)const{");
    output.add_header_line("return content[state][cell] >= current_threshold;");
    output.add_header_line("}");
    output.add_header_line("");
    output.add_header_line("inline void set(int state, int cell){");
    output.add_header_line("content[state][cell] = current_threshold;");
    output.add_header_line("}");
    output.add_header_line("");
    output.add_header_line("inline void reset(void){");
    output.add_header_line("if(current_threshold == std::numeric_limits<int>::max()){");
    output.add_header_line("for(unsigned int i=0;i<states;++i){");
    output.add_header_line("for(unsigned int j=0;j<cells;++j){");
    output.add_header_line("content[i][j] = std::numeric_limits<int>::min();");
    output.add_header_line("}");
    output.add_header_line("}");
    output.add_header_line("current_threshold = std::numeric_limits<int>::min()+1;");
    output.add_header_line("}");
    output.add_header_line("else{");
    output.add_header_line("++current_threshold;");
    output.add_header_line("}");
    output.add_header_line("}");
    output.add_header_line("");
    output.add_header_line("private:");
    output.add_header_line("int content[states][cells] = {};");
    output.add_header_line("int current_threshold = 1;");
    output.add_header_line("};");
    output.add_header_line("");
}

void game_compiler::generate_resettable_bitarray_stack(void){
    generate_resettable_bitarray();
    output.add_header_include("vector");
    output.add_header_line("class resettable_bitarray_stack{");
    output.add_header_line("public:");
    output.add_header_line("void push(void);");
    output.add_source_line("void resettable_bitarray_stack::push(void){");
    output.add_source_line("if(current_top >= main_content.size()){");
    output.add_source_line("main_content.emplace_back();");
    output.add_source_line("}");
    output.add_source_line("else{");
    output.add_source_line("main_content[current_top].reset();");
    output.add_source_line("}");
    output.add_source_line("++current_top;");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert_to_level(unsigned int level);");
    output.add_source_line("void resettable_bitarray_stack::revert_to_level(unsigned int level){");
    output.add_source_line("current_top = level;");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("unsigned int get_level(void);");
    output.add_source_line("unsigned int resettable_bitarray_stack::get_level(void){");
    output.add_source_line("return current_top;");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("bool is_set(int state, int cell)const;");
    output.add_source_line("bool resettable_bitarray_stack::is_set(int state, int cell)const{");
    output.add_source_line("return main_content[current_top-1].is_set(state,cell);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void set(int state, int cell);");
    output.add_source_line("void resettable_bitarray_stack::set(int state, int cell){");
    output.add_source_line("main_content[current_top-1].set(state,cell);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void reset(void);");
    output.add_source_line("void resettable_bitarray_stack::reset(void){");
    output.add_source_line("current_top = 0;");
    output.add_source_line("push();");
    output.add_source_line("}");
    output.add_source_line("");
    for(uint i=0;i<pattern_automata.size();++i){
        output.add_header_line("void pattern_push"+std::to_string(i)+"(void);");
        output.add_source_line("void resettable_bitarray_stack::pattern_push"+std::to_string(i)+"(void){");
        output.add_source_line("if(pattern_current_top"+std::to_string(i)+" >= pattern_content"+std::to_string(i)+".size()){");
        output.add_source_line("pattern_content"+std::to_string(i)+".emplace_back();");
        output.add_source_line("}");
        output.add_source_line("else{");
        output.add_source_line("pattern_content"+std::to_string(i)+"[pattern_current_top"+std::to_string(i)+"].reset();");
        output.add_source_line("}");
        output.add_source_line("++pattern_current_top"+std::to_string(i)+";");
        output.add_source_line("}");
        output.add_source_line("");
        output.add_header_line("void pattern_revert_to_level"+std::to_string(i)+"(unsigned int level);");
        output.add_source_line("void resettable_bitarray_stack::pattern_revert_to_level"+std::to_string(i)+"(unsigned int level){");
        output.add_source_line("pattern_current_top"+std::to_string(i)+" = level;");
        output.add_source_line("}");
        output.add_source_line("");
        output.add_header_line("unsigned int pattern_get_level"+std::to_string(i)+"(void);");
        output.add_source_line("unsigned int resettable_bitarray_stack::pattern_get_level"+std::to_string(i)+"(void){");
        output.add_source_line("return pattern_current_top"+std::to_string(i)+";");
        output.add_source_line("}");
        output.add_source_line("");
        output.add_header_line("bool pattern_is_set"+std::to_string(i)+"(int state, int cell)const;");
        output.add_source_line("bool resettable_bitarray_stack::pattern_is_set"+std::to_string(i)+"(int state, int cell)const{");
        output.add_source_line("return pattern_content"+std::to_string(i)+"[pattern_current_top"+std::to_string(i)+"-1].is_set(state,cell);");
        output.add_source_line("}");
        output.add_source_line("");
        output.add_header_line("void pattern_set"+std::to_string(i)+"(int state, int cell);");
        output.add_source_line("void resettable_bitarray_stack::pattern_set"+std::to_string(i)+"(int state, int cell){");
        output.add_source_line("pattern_content"+std::to_string(i)+"[pattern_current_top"+std::to_string(i)+"-1].set(state,cell);");
        output.add_source_line("}");
        output.add_source_line("");
        output.add_header_line("void pattern_reset"+std::to_string(i)+"(void);");
        output.add_source_line("void resettable_bitarray_stack::pattern_reset"+std::to_string(i)+"(void){");
        output.add_source_line("pattern_current_top"+std::to_string(i)+" = 0;");
        output.add_source_line("pattern_push"+std::to_string(i)+"();");
        output.add_source_line("}");
        output.add_source_line("");
    }
    output.add_header_line("private:");
    output.add_header_line("std::vector<resettable_bitarray<"+std::to_string(game_automaton.get_size())+","+std::to_string(input.get_board().get_size())+">> main_content = {};");
    output.add_header_line("unsigned int current_top = 0;");
    for(uint i=0;i<pattern_automata.size();++i){
        output.add_header_line("std::vector<resettable_bitarray<"+std::to_string(pattern_automata[i].get_size())+","+std::to_string(input.get_board().get_size())+">> pattern_content"+std::to_string(i)+" = {};");
        output.add_header_line("unsigned int pattern_current_top"+std::to_string(i)+" = 0;");
    }
    output.add_header_line("};");
    output.add_header_line("");
}

void game_compiler::generate_appliers_lists(void){
    output.add_header_include("memory");
    output.add_header_line("struct board_appliers{");
    output.add_header_line("board_changes_information head;");
    output.add_header_line("std::shared_ptr<board_appliers> tail;");
    output.add_header_line("board_appliers(int cell,int piece,const std::shared_ptr<board_appliers>& tail)");
    output.add_header_line(": head(cell,piece),");
    output.add_header_line("  tail(tail){");
    output.add_header_line("}");
    output.add_header_line("};");
    output.add_header_line("struct variables_appliers{");
    output.add_header_line("variable_changes_information head;");
    output.add_header_line("std::shared_ptr<variables_appliers> tail;");
    output.add_header_line("variables_appliers(int variable,int value,const std::shared_ptr<variables_appliers>& tail)");
    output.add_header_line(": head(variable,value),");
    output.add_header_line("  tail(tail){");
    output.add_header_line("}");
    output.add_header_line("};");
    output.add_header_line("");
}

void game_compiler::generate_move_class(void){
    output.add_header_include("memory");
    generate_appliers_lists();
    output.add_header_line("class move{");
    output.add_header_line("private:");
    output.add_header_line("std::shared_ptr<board_appliers> board_list;");
    output.add_header_line("std::shared_ptr<variables_appliers> variables_list;");
    output.add_header_line("int next_player;");
    output.add_header_line("int next_cell;");
    output.add_header_line("int next_state;");
    output.add_header_line("friend class next_states_iterator;");
    output.add_header_line("friend class game_state;");
    output.add_header_line("};");
}

const cpp_container& game_compiler::compile(void){
    build_game_automaton();
    output.add_header_line("namespace "+name+"{");
    output.add_source_line("namespace "+name+"{");
    generate_game_parameters();
    generate_board_cells_decoder();
    generate_pieces_decoder();
    generate_variables_decoder();
    output.add_header_line("");
    fill_edges_map();
    generate_board_structure();
    generate_variables_bounds();
    output.add_header_line("");
    generate_iterator_helper_structures();
    generate_move_class();
    generate_game_state_class();
    generate_states_iterator();
    output.add_header_line("}");
    output.add_source_line("}");
    return output;
}

