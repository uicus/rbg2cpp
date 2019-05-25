#include"game_compiler.hpp"
#include"compiler_options.hpp"
#include"automaton_builder.hpp"
#include"parsed_game.hpp"
#include"types.hpp"
#include"actions_compiler.hpp"
#include"transition_data.hpp"
#include<algorithm>

constexpr int MAXIMAL_GAME_DEPENDENT_STAIGHTNESS = 10;

game_compiler::game_compiler(const rbg_parser::parsed_game& input, const compiler_options& opts):
output(opts.output_file()),
opts(opts),
name(opts.output_file()),
pieces_to_id(),
edges_to_id(),
variables_to_id(),
states_to_bool_array(),
game_automaton(),
pattern_automata(),
shift_tables(),
precomputed_patterns(),
uses_pieces_in_arithmetics(input.get_moves()->has_piece_as_variable(input.get_declarations().get_legal_pieces())),
injective_board(input.get_board().is_injective()),
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
    output.add_source_line("return cell_neighbors[cell_id+1][edge_id]-1;");
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
    if(uses_pieces_in_arithmetics){
        std::string pieces_count_generated_array = "int pieces_count["+std::to_string(pieces_to_id.size())+"] = {";
        for(uint i=0;i<pieces_count.size();++i)
            pieces_count_generated_array += std::to_string(pieces_count[i]) + (i==pieces_count.size()-1?"":",");
        pieces_count_generated_array += "};";
        output.add_header_line(pieces_count_generated_array);
    }
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
    output.add_source_line("for(const auto& el: m.mr){");
    output.add_source_line("apply_action(el);");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("std::vector<move> get_all_moves(resettable_bitarray_stack& cache);");
    output.add_source_line("std::vector<move> game_state::get_all_moves(resettable_bitarray_stack& cache){");
    output.add_source_line("std::vector<move> result;");
    output.add_source_line("result.reserve(100);");
    output.add_source_line("get_all_moves(cache, result);");
    output.add_source_line("return result;");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void get_all_moves(resettable_bitarray_stack& cache, std::vector<move>& result);");
    output.add_source_line("void game_state::get_all_moves(resettable_bitarray_stack& cache, std::vector<move>& result){");
    output.add_source_line("result.clear();");
    output.add_source_line("next_states_iterator it(*this, cache, result);");
    output.add_source_line("it.get_all_moves(current_state, current_cell);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("bool apply_any_move(resettable_bitarray_stack& cache);");
    output.add_source_line("bool game_state::apply_any_move(resettable_bitarray_stack& cache){");
    output.add_source_line("std::vector<move> dummy;");
    output.add_source_line("next_states_iterator it(*this, cache, dummy);");
    output.add_source_line("return it.apply_any_move(current_state, current_cell);");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("friend class next_states_iterator;");
    output.add_header_line("private:");
    generate_actions_applier();
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
    std::vector<label> block;
    std::vector<label> shift_block;
    automaton_builder b(input.get_board(), pattern_automata, shift_tables, precomputed_patterns, block, shift_block, opts);
    input.get_moves()->accept(b);
    game_automaton = b.get_final_result();
    assert(block.empty() and shift_block.empty());
    game_automaton.mark_start_as_outgoing_usable();
    game_automaton.mark_states_as_double_reachable(shift_tables);
    game_automaton.add_information_about_states_to_see(states_to_bool_array);
    game_automaton.see_what_states_must_be_marked_by_move_enders();
    for(auto& el: pattern_automata){
        el.mark_start_as_outgoing_usable();
        el.mark_states_as_double_reachable(shift_tables);
        el.add_information_about_states_to_see(states_to_bool_array);
        el.see_what_states_must_be_marked_by_move_enders();
    }
    if(opts.enabled_any_square_optimisation())
        for(auto& el: shift_tables)
            el.check_if_any_square();
}

void game_compiler::generate_iterator_helper_structures(void){
    output.add_header_line("struct action_representation{");
    output.add_header_line("int index;");
    output.add_header_line("int cell;");
    output.add_header_line("action_representation(void)=default;");
    output.add_header_line("action_representation(int index,int cell)");
    output.add_header_line(": index(index),");
    output.add_header_line("  cell(cell){");
    output.add_header_line("}");
    output.add_header_line("};");
    output.add_header_line("");
}

void game_compiler::generate_main_next_getters(void){
    output.add_header_line("void get_all_moves(int state, int current_cell);");
    output.add_source_line("void next_states_iterator::get_all_moves(int state, int current_cell){");
    game_automaton.print_all_getters_table(output, "get_all_moves");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("bool apply_any_move(int state, int current_cell);");
    output.add_source_line("bool next_states_iterator::apply_any_move(int state, int current_cell){");
    game_automaton.print_any_appliers_table(output, "apply_any_move");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_pattern_evaluator(uint pattern_index){
    output.add_header_line("bool evaluate"+std::to_string(pattern_index)+"(int current_cell);");
    output.add_source_line("bool next_states_iterator::evaluate"+std::to_string(pattern_index)+"(int current_cell){");
    output.add_source_line("cache.pattern_reset"+std::to_string(pattern_index)+"();");
    static_transition_data static_data(
        pieces_to_id,
        edges_to_id,
        variables_to_id,
        states_to_bool_array,
        input.get_declarations(),
        shift_tables,
        precomputed_patterns,
        uses_pieces_in_arithmetics,
        injective_board,
        "get_pattern_value"+std::to_string(pattern_index)+"_",
        inside_pattern,
        pattern_index);
    pattern_automata[pattern_index].print_recursive_calls_for_pattern_in_start_state(
        output,
        static_data);
    output.add_source_line("return false;");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_states_iterator(void){
    output.add_header_include("vector");
    output.add_header_line("class next_states_iterator{");
    output.add_header_line("public:");
    output.add_header_line("next_states_iterator(game_state& state_to_change, resettable_bitarray_stack& cache, std::vector<move>& moves);");
    output.add_source_line("next_states_iterator::next_states_iterator(game_state& state_to_change, resettable_bitarray_stack& cache, std::vector<move>& moves)");
    output.add_source_line(": state_to_change(state_to_change),");
    output.add_source_line("  cache(cache),");
    output.add_source_line("  moves(moves){");
    output.add_source_line("cache.reset();");
    output.add_source_line("}");
    output.add_source_line("");
    generate_main_next_getters();
    output.add_header_line("private:");
    for(uint i=0;i<pattern_automata.size();++i)
        generate_pattern_evaluator(i);
    game_automaton.print_transition_functions(
        output,
        static_transition_data(
            pieces_to_id,
            edges_to_id,
            variables_to_id,
            states_to_bool_array,
            input.get_declarations(),
            shift_tables,
            precomputed_patterns,
            uses_pieces_in_arithmetics,
            injective_board,
            "apply_any_move_",
            any_getter));
    game_automaton.print_transition_functions(
        output,
        static_transition_data(
            pieces_to_id,
            edges_to_id,
            variables_to_id,
            states_to_bool_array,
            input.get_declarations(),
            shift_tables,
            precomputed_patterns,
            uses_pieces_in_arithmetics,
            injective_board,
            "get_all_moves_",
            all_getter));
    for(uint i=0;i<pattern_automata.size();++i){
        output.add_header_line("");
        pattern_automata[i].print_transition_functions(
            output,
            static_transition_data(
                pieces_to_id,
                edges_to_id,
                variables_to_id,
                states_to_bool_array,
                input.get_declarations(),
                shift_tables,
                precomputed_patterns,
                uses_pieces_in_arithmetics,
                injective_board,
                "get_pattern_value"+std::to_string(i)+"_",
                inside_pattern,
                i));
    }
    output.add_header_line("");
    output.add_header_line("game_state& state_to_change;");
    output.add_header_line("resettable_bitarray_stack& cache;");
    output.add_header_line("move_representation mr;");
    output.add_header_line("std::vector<move>& moves;");
    generate_visited_array_for_prioritized_states();
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
    output.add_header_include("boost/container/small_vector.hpp");
    int straightness = input.get_moves()->compute_k_straightness().final_result();
    output.add_header_line("typedef boost::container::small_vector<action_representation, "+std::to_string((straightness<MAXIMAL_GAME_DEPENDENT_STAIGHTNESS and straightness>0?straightness:MAXIMAL_GAME_DEPENDENT_STAIGHTNESS)+1)+"> move_representation;");
}

void game_compiler::generate_actions_applier(void){
    static_transition_data static_data(
        pieces_to_id,
        edges_to_id,
        variables_to_id,
        states_to_bool_array,
        input.get_declarations(),
        shift_tables,
        precomputed_patterns,
        uses_pieces_in_arithmetics,
        injective_board,
        "",
        all_getter);

    output.add_header_line("void apply_action(const action_representation& action);");
    output.add_source_line("void game_state::apply_action(const action_representation& action){");
    output.add_source_line("switch(action.index){");
    game_automaton.print_indices_to_actions_correspondence(output,static_data);
    output.add_source_line("default:");
    output.add_source_line("break;");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_move_class(void){
    generate_appliers_lists();
    output.add_header_line("class move{");
    output.add_header_line("move_representation mr;");
    output.add_header_line("friend class next_states_iterator;");
    output.add_header_line("friend class game_state;");
    output.add_header_line("public:");
    output.add_header_line("move(void) = default;");
    output.add_header_line("move(const move_representation& mr)");
    output.add_header_line(": mr(mr){");
    output.add_header_line("}");
    output.add_header_line("};");
}

void game_compiler::print_all_shift_tables(void){
    for(uint i=0;i<shift_tables.size();++i)
        shift_tables[i].print_array(output,i);
}

void game_compiler::generate_visited_array_for_prioritized_states(void){
    if(not states_to_bool_array.empty())
        output.add_header_line("bool visited_for_prioritized["+std::to_string(states_to_bool_array.size())+"];");
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
    print_all_shift_tables();
    output.add_header_line("");
    generate_resettable_bitarray_stack();
    generate_iterator_helper_structures();
    generate_move_class();
    generate_game_state_class();
    generate_states_iterator();
    output.add_header_line("}");
    output.add_source_line("}");
    return output;
}

