#include"game_compiler.hpp"
#include"automaton_builder.hpp"
#include"parsed_game.hpp"
#include"types.hpp"
#include<algorithm>

game_compiler::game_compiler(const rbg_parser::parsed_game& input, const std::string& output_name):
output(output_name),
name(output_name),
pieces_to_id(),
game_automaton(),
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
    for(uint i=0;i<g.get_size();++i)
        output.add_header_line(std::to_string(pieces_to_id[g.get_starting_piece(i)])+",");
    output.add_header_line("};");
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
    output.add_header_line("friend class next_states_iterator;");
    output.add_header_line("private:");
    output.add_header_line("int current_cell = 1;");
    output.add_header_line("int current_player = 0;");
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
    output.add_header_line("constexpr int board_size = "+std::to_string(g.get_size())+";");
    output.add_header_line("constexpr int number_of_players = "+std::to_string(pl.size()+1)+";");
    output.add_header_line("constexpr int number_of_pieces = "+std::to_string(pi.size())+";");
    output.add_header_line("constexpr int number_of_variables = "+std::to_string(pl.size()+v.size())+";");
    output.add_header_line("constexpr int board_degree = "+std::to_string(e.size())+";");
    output.add_header_line("");
}

void game_compiler::build_game_automaton(void){
    automaton_builder b;
    input.get_moves()->accept(b);
    game_automaton = b.get_final_result();
}

void game_compiler::generate_iterator_helper_structures(void){
    output.add_header_line("struct backtrace_information{");
    output.add_header_line("unsigned int current_branch;");
    output.add_header_line("int state_checkpoint;");
    output.add_header_line("int current_cell_checkpoint;");
    output.add_header_line("unsigned int modifiers_depth_checkpoint;");
    output.add_header_line("unsigned int board_checkpoint;");
    output.add_header_line("unsigned int variables_checkpoint;");
    output.add_header_line("};");
    output.add_header_line("struct board_revert_information{");
    output.add_header_line("int cell;");
    output.add_header_line("int previous_piece;");
    output.add_header_line("};");
    output.add_header_line("struct variable_revert_information{");
    output.add_header_line("int variable;");
    output.add_header_line("int previous_value;");
    output.add_header_line("};");
    output.add_header_line("");
}

void game_compiler::generate_iterator_revert_methods(void){
    output.add_header_line("void revert_board_changes(unsigned int to_point);");
    output.add_source_line("void next_states_iterator::revert_board_changes(unsigned int to_point){");
    output.add_source_line("for(unsigned int i=board_change_points.size()-1;i>=to_point;--i){");
    output.add_source_line("state_to_change.pieces[board_change_points[i].cell] = board_change_points[i].previous_piece;");
    output.add_source_line("}");
    output.add_source_line("board_change_points.resize(to_point);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert_variables_changes(unsigned int to_point);");
    output.add_source_line("void next_states_iterator::revert_variables_changes(unsigned int to_point){");
    output.add_source_line("for(unsigned int i=variables_change_points.size()-1;i>=to_point;--i){");
    output.add_source_line("state_to_change.variables[variables_change_points[i].variable] = variables_change_points[i].previous_value;");
    output.add_source_line("}");
    output.add_source_line("variables_change_points.resize(to_point);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert_to_point(unsigned int decision_point);");
    output.add_source_line("void next_states_iterator::revert_to_point(unsigned int decision_point){");
    output.add_source_line("const auto& point_to_revert = decision_points[decision_point];");
    output.add_source_line("state_to_change.current_cell = point_to_revert.current_cell_checkpoint;");
    output.add_source_line("cache.revert_to_level(point_to_revert.modifiers_depth_checkpoint);");
    output.add_source_line("revert_board_changes(point_to_revert.board_checkpoint);");
    output.add_source_line("revert_variables_changes(point_to_revert.variables_checkpoint);");
    output.add_source_line("decision_points.resize(decision_point);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert_to_state(int state_id);");
    output.add_source_line("void next_states_iterator::revert_to_state(int state_id){");
    output.add_source_line("for(auto i=decision_points.size();i>0;--i){");
    output.add_source_line("if(decision_points[i-1].state_checkpoint == state_id){");
    output.add_source_line("revert_to_point(i-1);");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_states_iterator(void){
    generate_resettable_bitarray_stack();
    output.add_header_include("vector");
    output.add_header_line("class next_states_iterator{");
    output.add_header_line("public:");
    output.add_header_line("next_states_iterator(game_state& state_to_change, resettable_bitarray_stack& cache);");
    output.add_source_line("next_states_iterator::next_states_iterator(game_state& state_to_change, resettable_bitarray_stack& cache)");
    output.add_source_line(": state_to_change(state_to_change),");
    output.add_source_line("  cache(cache){}");
    output.add_source_line("");
    output.add_header_line("private:");
    generate_iterator_helper_structures();
    generate_iterator_revert_methods();
    output.add_header_line("game_state& state_to_change;");
    output.add_header_line("resettable_bitarray_stack& cache;");
    output.add_header_line("std::vector<backtrace_information> decision_points;");
    output.add_header_line("std::vector<board_revert_information> board_change_points;");
    output.add_header_line("std::vector<variable_revert_information> variables_change_points;");
    output.add_header_line("};");
}

void game_compiler::generate_resettable_bitarray(){
    output.add_header_include("limits");
    output.add_header_line("class resettable_bitarray{");
    output.add_header_line("public:");
    output.add_header_line("bool is_set(int state, int cell)const;");
    output.add_source_line("bool resettable_bitarray::is_set(int state, int cell)const{");
    output.add_source_line("return content[state][cell] >= current_threshold;");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void set(int state, int cell);");
    output.add_source_line("void resettable_bitarray::set(int state, int cell){");
    output.add_source_line("content[state][cell] = current_threshold;");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void reset(void);");
    output.add_source_line("void resettable_bitarray::reset(void){");
    output.add_source_line("if(current_threshold == std::numeric_limits<int>::max()){");
    output.add_source_line("for(unsigned int i=0;i<"+std::to_string(game_automaton.get_size())+";++i){");
    output.add_source_line("for(unsigned int j=0;j<"+std::to_string(input.get_board().get_size())+";++j){");
    output.add_source_line("content[i][j] = std::numeric_limits<int>::min();");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("current_threshold = std::numeric_limits<int>::min()+1;");
    output.add_source_line("}");
    output.add_source_line("else{");
    output.add_source_line("++current_threshold;");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("private:");
    output.add_header_line("int content["+std::to_string(game_automaton.get_size())+"]["+std::to_string(input.get_board().get_size())+"] = {};");
    output.add_header_line("int current_threshold = std::numeric_limits<int>::min();");
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
    output.add_source_line("if(current_top >= content.size()){");
    output.add_source_line("content.emplace_back();");
    output.add_source_line("}");
    output.add_source_line("else{");
    output.add_source_line("content[current_top].reset();");
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
    output.add_source_line("return content[current_top-1].is_set(state,cell);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void set(int state, int cell);");
    output.add_source_line("void resettable_bitarray_stack::set(int state, int cell){");
    output.add_source_line("content[current_top-1].set(state,cell);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void reset(void);");
    output.add_source_line("void resettable_bitarray_stack::reset(void){");
    output.add_source_line("current_top = 0;");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("private:");
    output.add_header_line("std::vector<resettable_bitarray> content = {};");
    output.add_header_line("unsigned int current_top = 0;");
    output.add_header_line("};");
    output.add_header_line("");
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
    generate_game_state_class();
    generate_states_iterator();
    output.add_header_line("}");
    output.add_source_line("}");
    return output;
}

