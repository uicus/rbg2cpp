#include"game_compiler.hpp"
#include"compiler_options.hpp"
#include"automaton_builder.hpp"
#include"parsed_game.hpp"
#include"types.hpp"
#include"actions_compiler.hpp"
#include"transition_data.hpp"
#include"rules_board_automaton.hpp"
#include"modifiers_counter.hpp"
#include"monotonicity_determiner.hpp"
#include"monotonic_move.hpp"
#include<algorithm>

constexpr int MAXIMAL_GAME_DEPENDENT_STAIGHTNESS = 10;

game_compiler::game_compiler(const rbg_parser::parsed_game& input, const compiler_options& opts):
output(opts.output_file()),
opts(opts),
name(opts.output_file()),
pieces_to_id(),
edges_to_id(),
variables_to_id(),
board_structure(),
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
    const auto& p = input.get_declarations().get_legal_players_ordered();
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

std::string game_compiler::numbers_to_array(const std::vector<uint>& numbers)const{
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
    output.add_source_line(numbers_to_array(std::vector<uint>(edges_to_id.size(),0))+",");
    for(uint i=0;i<g.get_size();++i){
        const auto& outgoing_edges = g.get_outgoing_edges(i);
        std::vector<uint> neighbors(edges_to_id.size(),0);
        for(const auto& el: outgoing_edges)
            neighbors[edges_to_id[el.first]] = int(el.second+1);
        output.add_source_line(numbers_to_array(neighbors)+",");
        board_structure.emplace_back(std::move(neighbors));
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
    output.add_header_line("int get_current_state_id(void)const;");
    output.add_source_line("int game_state::get_current_state_id(void)const{");
    output.add_source_line("return current_state;");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("int get_player_score(int player_id)const;");
    output.add_source_line("int game_state::get_player_score(int player_id)const{");
    output.add_source_line("return variables[player_id-1];");
    output.add_source_line("}");
    output.add_source_line("");
    if(opts.enabled_semisplit())
        game_automaton.print_is_nodal_function(output);
}

void game_compiler::generate_game_state_class(void){
    output.add_header_line("class game_state{");
    output.add_header_line("public:");
    generate_state_getters();
    output.add_header_line("void apply(const move& m){apply_move(m);}");
    output.add_header_line("void apply_move(const move& m);");
    output.add_source_line("void game_state::apply_move(const move& m){");
    output.add_source_line("for(auto it=m.mr.cbegin(); it!=m.mr.cend()-1; it++){");
    output.add_source_line("fast_apply_action(*it);");
    output.add_source_line("}");
    output.add_source_line("apply_action(m.mr.back());");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("move_reverter apply_with_revert(const move& m){return apply_move_with_revert(m);}");
    output.add_header_line("move_reverter apply_move_with_revert(const move& m);");
    output.add_source_line("move_reverter game_state::apply_move_with_revert(const move& m){");
    output.add_source_line("move_reverter moverev;");
    output.add_source_line("moverev.previous_state = current_state;");
    output.add_source_line("for(auto it=m.mr.cbegin(); it!=m.mr.cend()-1; it++){");
    output.add_source_line("moverev.modrevs.push_back(fast_apply_action_with_revert(*it));");
    output.add_source_line("}");
    output.add_source_line("moverev.modrevs.push_back(apply_action_with_revert(m.mr.back()).modrev);");
    output.add_source_line("return moverev;");
    output.add_source_line("}");
    output.add_source_line("");
    generate_main_next_getters();
    generate_reverter();
    generate_monotonic_moves();
    generate_actions_applier();
    output.add_header_line("private:");
    generate_states_iterator();
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
    output.add_header_line("constexpr int AUTOMATON_SIZE = "+std::to_string(game_automaton.get_size())+";");
    output.add_header_line("constexpr int NUMBER_OF_PLAYERS = "+std::to_string(pl.size()+1)+";");
    output.add_header_line("constexpr int NUMBER_OF_PIECES = "+std::to_string(pi.size())+";");
    output.add_header_line("constexpr int NUMBER_OF_VARIABLES = "+std::to_string(pl.size()+v.size())+";");
    output.add_header_line("constexpr int BOARD_DEGREE = "+std::to_string(e.size())+";");
    output.add_header_line("constexpr int MONOTONIC_CLASSES = "+std::to_string(monotonic_moves.size())+";");
    int move_straightness = input.get_moves()->compute_k_straightness(rbg_parser::StraightnessType::MOVE_STRAIGHTNESS).final_result(rbg_parser::StraightnessType::MOVE_STRAIGHTNESS);
    if (move_straightness >= 0) move_straightness++;
    output.add_header_line("constexpr int MOVE_STRAIGHTNESS = "+std::to_string(move_straightness)+";");
    output.add_header_line("");
}

void game_compiler::build_game_automaton(void){
    std::vector<label> block;
    std::vector<label> shift_block;
    automaton_builder b(input.get_board(), pattern_automata, shift_tables, precomputed_patterns, block, shift_block, opts);
    input.get_moves()->accept(b);
    game_automaton = b.get_final_result();
    assert(block.empty() and shift_block.empty());
    game_automaton.mark_start_as_rules_beginning();
    game_automaton.mark_states_as_double_reachable(shift_tables);
    for(auto& el: pattern_automata){
        el.mark_start_as_rules_beginning();
        el.mark_states_as_double_reachable(shift_tables);
    }
    if(opts.enabled_any_square_optimisation())
        for(auto& el: shift_tables)
            el.check_if_any_square();
    monotonic_moves = game_automaton.get_monotonics(shift_tables);
}

void game_compiler::generate_cache_checks_container(void){
    ccc = cache_checks_container(game_automaton,
                                 pattern_automata,
                                 shift_tables,
                                 precomputed_patterns,
                                 board_structure,
                                 edges_to_id,
                                 opts.enabled_cache_optimisation());
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
    output.add_header_line("bool operator==(const action_representation& rhs) const;");
    output.add_source_line("bool action_representation::operator==(const action_representation& rhs) const{");
    output.add_source_line("return index == rhs.index and cell == rhs.cell;");
    output.add_source_line("}");
    output.add_header_line("};");
    output.add_header_line("");
}

void game_compiler::generate_main_next_getters(void){
    // apply_any_move (always)
    output.add_header_line("bool apply_any_move(resettable_bitarray_stack&"+std::string(ccc.is_any_cache_needed()?" cache":"")+");");
    output.add_source_line("bool game_state::apply_any_move(resettable_bitarray_stack&"+std::string(ccc.is_any_cache_needed()?" cache":"")+"){");
    if(ccc.is_main_cache_needed())
        output.add_source_line("cache.reset();");
    game_automaton.print_any_appliers_table(output, "apply_any_move", ccc.is_any_cache_needed());
    output.add_source_line("}");
    output.add_source_line("");

    if (opts.enabled_moves_getter()) {// get_all_moves
        output.add_header_line("void get_all_moves(resettable_bitarray_stack&"+std::string(ccc.is_any_cache_needed()?" cache":"")+", std::vector<move>& moves);");
        output.add_source_line("void game_state::get_all_moves(resettable_bitarray_stack&"+std::string(ccc.is_any_cache_needed()?" cache":"")+", std::vector<move>& moves){");
        if(ccc.is_main_cache_needed())
            output.add_source_line("cache.reset();");
        output.add_source_line("moves.clear();");
        output.add_source_line("move_representation mr;");
        game_automaton.print_all_getters_table(output, "get_all_moves", ccc.is_any_cache_needed(), mode::semisplit_off);
        output.add_source_line("}");
        output.add_source_line("");
    }
    if(opts.enabled_actions_getter()){// get_all_actions
        output.add_header_line("void get_all_actions(resettable_bitarray_stack&"+std::string(ccc.is_any_cache_needed()?" cache":"")+", std::vector<action_representation>& moves);");
        output.add_source_line("void game_state::get_all_actions(resettable_bitarray_stack&"+std::string(ccc.is_any_cache_needed()?" cache":"")+", std::vector<action_representation>& moves){");
        if(ccc.is_main_cache_needed())
            output.add_source_line("cache.reset();");
        output.add_source_line("moves.clear();");
        game_automaton.print_all_getters_table(output, "get_all_actions", ccc.is_any_cache_needed(), mode::semisplit_actions);
        output.add_source_line("}");
        output.add_source_line("");
    }
    if(opts.enabled_dotsplit_getter()){// get_all_semimoves
        output.add_header_line("void get_all_semimoves(resettable_bitarray_stack&"+std::string(ccc.is_any_cache_needed()?" cache":"")+", std::vector<move>& moves);");
        output.add_source_line("void game_state::get_all_semimoves(resettable_bitarray_stack&"+std::string(ccc.is_any_cache_needed()?" cache":"")+", std::vector<move>& moves){");
        if(ccc.is_main_cache_needed())
            output.add_source_line("cache.reset();");
        output.add_source_line("moves.clear();");
        output.add_source_line("move_representation mr;");
        game_automaton.print_all_getters_table(output, "get_all_semimoves", ccc.is_any_cache_needed(), mode::semisplit_dotsplit);
        output.add_source_line("}");
        output.add_source_line("");
    }
}

void game_compiler::generate_pattern_evaluator(uint pattern_index){
    output.add_header_line("bool evaluate"+std::to_string(pattern_index)+"(int cell"+std::string(ccc.is_any_cache_needed()?", resettable_bitarray_stack& cache":"")+");");
    output.add_source_line("bool game_state::evaluate"+std::to_string(pattern_index)+"(int cell"+std::string(ccc.is_any_cache_needed()?", resettable_bitarray_stack& cache":"")+"){");
    if(ccc.is_pattern_cache_needed(pattern_index))
        output.add_source_line("cache.pattern_reset"+std::to_string(pattern_index)+"();");
    static_transition_data static_data(
        opts,
        mode::semisplit_off,
        pieces_to_id,
        edges_to_id,
        variables_to_id,
        input.get_declarations(),
        shift_tables,
        precomputed_patterns,
        ccc,
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
    for(uint i=0;i<pattern_automata.size();++i)
        generate_pattern_evaluator(i);
    game_automaton.print_transition_functions(
        output,
        static_transition_data(
            opts,
            mode::semisplit_off,
            pieces_to_id,
            edges_to_id,
            variables_to_id,
            input.get_declarations(),
            shift_tables,
            precomputed_patterns,
            ccc,
            uses_pieces_in_arithmetics,
            injective_board,
            "apply_any_move_",
            any_getter));
    if (opts.enabled_moves_getter()) {
        game_automaton.print_transition_functions(
            output,
            static_transition_data(
                opts,
                mode::semisplit_off,
                pieces_to_id,
                edges_to_id,
                variables_to_id,
                input.get_declarations(),
                shift_tables,
                precomputed_patterns,
                ccc,
                uses_pieces_in_arithmetics,
                injective_board,
                "get_all_moves_",
                all_getter));
    }
    if (opts.enabled_actions_getter()) {
        game_automaton.print_transition_functions(
            output,
            static_transition_data(
                opts,
                mode::semisplit_actions,
                pieces_to_id,
                edges_to_id,
                variables_to_id,
                input.get_declarations(),
                shift_tables,
                precomputed_patterns,
                ccc,
                uses_pieces_in_arithmetics,
                injective_board,
                "get_all_actions_",
                all_getter));
    }
    if (opts.enabled_dotsplit_getter()) {
        game_automaton.print_transition_functions(
            output,
            static_transition_data(
                opts,
                mode::semisplit_dotsplit,
                pieces_to_id,
                edges_to_id,
                variables_to_id,
                input.get_declarations(),
                shift_tables,
                precomputed_patterns,
                ccc,
                uses_pieces_in_arithmetics,
                injective_board,
                "get_all_semimoves_",
                all_getter));
    }
    for(uint i=0;i<pattern_automata.size();++i){
        output.add_header_line("");
        pattern_automata[i].print_transition_functions(
            output,
            static_transition_data(
                opts,
                mode::semisplit_off,
                pieces_to_id,
                edges_to_id,
                variables_to_id,
                input.get_declarations(),
                shift_tables,
                precomputed_patterns,
                ccc,
                uses_pieces_in_arithmetics,
                injective_board,
                "get_pattern_value"+std::to_string(i)+"_",
                inside_pattern,
                i));
    }
    output.add_header_line("");
}

void game_compiler::generate_appliers_lists(void){
    int straightness = input.get_moves()->compute_k_straightness(rbg_parser::StraightnessType::MOVE_STRAIGHTNESS).final_result(rbg_parser::StraightnessType::MOVE_STRAIGHTNESS);
    if(straightness<MAXIMAL_GAME_DEPENDENT_STAIGHTNESS and straightness>=0){
        output.add_header_include("boost/container/static_vector.hpp");
        output.add_header_line("using move_representation = boost::container::static_vector<action_representation, MOVE_STRAIGHTNESS>;");
    }
    else{
        output.add_header_include("boost/container/small_vector.hpp");
        output.add_header_line("using move_representation = boost::container::small_vector<action_representation, "+std::to_string(MAXIMAL_GAME_DEPENDENT_STAIGHTNESS+1)+">;");
    }
}

void game_compiler::generate_revert_info_structure(void){
    output.add_header_line("struct mod_reverter{");
    output.add_header_line("int action_index;");
    output.add_header_line("int previous_value;");
    output.add_header_line("int previous_cell;");
    output.add_header_line("friend class game_state;");
    output.add_header_line("};");
    output.add_header_line("");
    output.add_header_line("struct action_reverter{");
    output.add_header_line("int previous_state;");
    output.add_header_line("mod_reverter modrev;");
    output.add_header_line("friend class game_state;");
    output.add_header_line("};");
    output.add_header_line("");
    output.add_header_line("struct move_reverter{");
    output.add_header_line("int previous_state;");
    int straightness = input.get_moves()->compute_k_straightness(rbg_parser::StraightnessType::MOVE_STRAIGHTNESS).final_result(rbg_parser::StraightnessType::MOVE_STRAIGHTNESS);
    if(straightness<MAXIMAL_GAME_DEPENDENT_STAIGHTNESS and straightness>=0){
        output.add_header_line("boost::container::static_vector<mod_reverter, MOVE_STRAIGHTNESS> modrevs;");
    } else {
        output.add_header_line("boost::container::small_vector<mod_reverter, " + std::to_string(MAXIMAL_GAME_DEPENDENT_STAIGHTNESS+1) + "> modrevs;");
    }
    output.add_header_line("friend class game_state;");
    output.add_header_line("};");
    output.add_header_line("");
}

void game_compiler::generate_actions_applier(void){
    static_transition_data static_data(
        opts,
        mode::semisplit_off,
        pieces_to_id,
        edges_to_id,
        variables_to_id,
        input.get_declarations(),
        shift_tables,
        precomputed_patterns,
        ccc,
        uses_pieces_in_arithmetics,
        injective_board,
        "",
        all_getter);
    output.add_header_line("void apply(const action_representation action){apply_action(action);}");
    output.add_header_line("void apply_action(const action_representation action);");
    output.add_source_line("void game_state::apply_action(const action_representation action){");
    output.add_source_line("switch(action.index){");
    game_automaton.print_indices_to_actions_correspondence(output,static_data,false,false,true);
    output.add_source_line("default:");
    output.add_source_line("current_state = -action.index;");
    output.add_source_line("current_cell = action.cell;");
    output.add_source_line("break;");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_source_line("");
    output.add_header_line("void fast_apply_action(const action_representation action);");
    output.add_source_line("void game_state::fast_apply_action(const action_representation action){");
    output.add_source_line("switch(action.index){");
    game_automaton.print_indices_to_actions_correspondence(output,static_data,false,false,false);
    output.add_source_line("default:");
    output.add_source_line("break;");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("action_reverter apply_with_revert(const action_representation action){return apply_action_with_revert(action);}");
    output.add_header_line("action_reverter apply_action_with_revert(const action_representation action);");
    output.add_source_line("action_reverter game_state::apply_action_with_revert(const action_representation action){");
    output.add_source_line("action_reverter acrev;");
    output.add_source_line("acrev.previous_state = current_state;");
    output.add_source_line("mod_reverter &modrev = acrev.modrev;");
    output.add_source_line("modrev.previous_cell = current_cell;");
    output.add_source_line("modrev.action_index = action.index;");
    output.add_source_line("switch(action.index){");
    game_automaton.print_indices_to_actions_correspondence(output,static_data,false,true,true);
    output.add_source_line("default:");
    output.add_source_line("current_state = -action.index;");
    output.add_source_line("current_cell = action.cell;");
    output.add_source_line("break;");
    output.add_source_line("}");
    output.add_source_line("return acrev;");
    output.add_source_line("}");
    output.add_header_line("mod_reverter fast_apply_action_with_revert(const action_representation action);");
    output.add_source_line("mod_reverter game_state::fast_apply_action_with_revert(const action_representation action){");
    output.add_source_line("mod_reverter modrev;");
    output.add_source_line("modrev.previous_cell = current_cell;");
    output.add_source_line("modrev.action_index = action.index;");
    output.add_source_line("switch(action.index){");
    game_automaton.print_indices_to_actions_correspondence(output,static_data,false,true,false);
    output.add_source_line("default:");
    output.add_source_line("break;");
    output.add_source_line("}");
    output.add_source_line("return modrev;");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_reverter(void){
    static_transition_data static_data(
    opts,
    mode::semisplit_off,
    pieces_to_id,
    edges_to_id,
    variables_to_id,
    input.get_declarations(),
    shift_tables,
    precomputed_patterns,
    ccc,
    uses_pieces_in_arithmetics,
    injective_board,
    "",
    all_getter);
    output.add_header_line("void revert(const mod_reverter modrev);");
    output.add_source_line("void game_state::revert(const mod_reverter modrev){");
    output.add_source_line("switch(modrev.action_index){");
    game_automaton.print_indices_to_actions_correspondence(output,static_data,true,false,false);
    output.add_source_line("default:");
    output.add_source_line("current_cell = modrev.previous_cell;");    
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert(const action_reverter& actionrev){revert_action(actionrev);}");
    output.add_header_line("void revert_action(const action_reverter& actionrev);");
    output.add_source_line("void game_state::revert_action(const action_reverter& actionrev){");
    output.add_source_line("revert(actionrev.modrev);");
    output.add_source_line("current_state = actionrev.previous_state;");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("void revert(const move_reverter& moverev){revert_move(moverev);}");
    output.add_header_line("void revert_move(const move_reverter& moverev);");
    output.add_source_line("void game_state::revert_move(const move_reverter& moverev){");
    output.add_source_line("for(auto it=moverev.modrevs.cend(); it!=moverev.modrevs.cbegin();){");
    output.add_source_line("it--;");
    output.add_source_line("revert(*it);");
    output.add_source_line("}");
    output.add_source_line("current_state = moverev.previous_state;");
    output.add_source_line("}");
    output.add_source_line("");
}

void game_compiler::generate_move_class(void){
    generate_appliers_lists();
    output.add_header_line("struct move{");
    output.add_header_line("move_representation mr;");
    output.add_header_line("move(void) = default;");
    output.add_header_line("move(const move_representation& mr);");
    output.add_source_line("move::move(const move_representation& mr)");
    output.add_source_line(": mr(mr){");
    output.add_source_line("}");
    output.add_header_line("bool operator==(const move& rhs) const;");
    output.add_source_line("bool move::operator==(const move& rhs) const{");
    output.add_source_line("return mr == rhs.mr;");
    output.add_source_line("}");
    output.add_header_line("};");
}

void game_compiler::print_all_shift_tables(void){
    for(uint i=0;i<shift_tables.size();++i)
        shift_tables[i].print_array(output,i);
}

void game_compiler::generate_indices_converters(void){
    modifiers_counter mc;
    input.get_moves()->accept(mc);
    auto modifiers_count_to_actions_count = mc.get_result();
    output.add_header_line("constexpr int NUMBER_OF_MODIFIERS = "+std::to_string(modifiers_count_to_actions_count.size())+";");
    auto switches_indices = mc.get_switches_indices();
    output.add_header_line("int is_switch(int action_index);");
    output.add_source_line("int is_switch(int action_index){");
    output.add_source_line("switch(action_index){");
    for(const auto switch_index: switches_indices)
        output.add_source_line("case "+std::to_string(switch_index)+":");
    output.add_source_line("return true;");
    output.add_source_line("default:");
    output.add_source_line("return false;");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("");

}

void game_compiler::generate_monotonic_moves(void){
    output.add_header_line("int get_monotonicity_class(void);");
    output.add_source_line("int game_state::get_monotonicity_class(void){");
    output.add_source_line("switch(current_state){");
    for(uint i=0;i<monotonic_moves.size();++i){
        output.add_source_line("case "+std::to_string(monotonic_moves[i].start_state)+":");
        output.add_source_line("return "+std::to_string(i)+";");
    }
    output.add_source_line("default:");
    output.add_source_line("return -1;");
    output.add_source_line("}");
    output.add_source_line("}");
    output.add_source_line("");
    output.add_header_line("bool is_legal(const move& m)const;");
    output.add_source_line("bool game_state::is_legal([[maybe_unused]] const move& m)const{");
    if(opts.enabled_safe_monotonicity_methods()){
        output.add_source_line("if(m.mr.size() != 1){");
        output.add_source_line("return false;");
        output.add_source_line("}");
    }
    output.add_source_line("switch(current_state){");
    static_transition_data static_data(
        opts,
        mode::semisplit_off,
        pieces_to_id,
        edges_to_id,
        variables_to_id,
        input.get_declarations(),
        shift_tables,
        precomputed_patterns,
        ccc,
        uses_pieces_in_arithmetics,
        injective_board,
        "",
        all_getter);
    for(const auto& el: monotonic_moves){
        output.add_source_line("case "+std::to_string(el.start_state)+":");
        el.print_legality_check(output, static_data);
        output.add_source_line("return true;");
    }
    output.add_source_line("default:");
    output.add_source_line("return false;");
    output.add_source_line("}");
    output.add_source_line("}");
}

const cpp_container& game_compiler::compile(void){
    build_game_automaton();
    output.add_header_line("namespace "+name+"{");
    output.add_source_line("namespace "+name+"{");
    generate_game_parameters();
    generate_board_cells_decoder();
    generate_pieces_decoder();
    generate_variables_decoder();
    generate_indices_converters();
    output.add_header_line("");
    fill_edges_map();
    generate_board_structure();
    generate_cache_checks_container();
    generate_variables_bounds();
    print_all_shift_tables();
    output.add_header_line("");
    ccc.generate_resettable_bitarray_stack(output);
    generate_iterator_helper_structures();
    generate_move_class();
    generate_revert_info_structure();
    generate_game_state_class();
    output.add_header_line("}");
    output.add_source_line("}");
    return output;
}

