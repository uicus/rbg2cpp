#include"cache_checks_container.hpp"
#include"automaton.hpp"
#include"rules_board_automaton.hpp"
#include"cpp_container.hpp"
#include<cassert>

cache_checks_container::cache_checks_container(const automaton& main_automaton,
                                               const std::vector<automaton>& pattern_automata,
                                               const std::vector<shift_table>& shift_tables,
                                               const std::vector<precomputed_pattern>& precomputed_patterns,
                                               const std::vector<std::vector<uint>>& board_structure,
                                               const std::map<rbg_parser::token, uint>& edges_to_id,
                                               bool enable_optimisation)
  : board_size(board_structure.size()){
    if(enable_optimisation){
        auto main_rba = main_automaton.generate_rules_board_automaton(shift_tables,
                                                                      precomputed_patterns,
                                                                      board_structure,
                                                                      edges_to_id);
        main_automation_checks = transform_mask_into_cache_info(main_rba.get_cache_checks_need());
        for(const auto& el: pattern_automata){
            auto pattern_rba = el.generate_rules_board_automaton(shift_tables,
                                                                 precomputed_patterns,
                                                                 board_structure,
                                                                 edges_to_id);
            pattern_automata_checks.emplace_back(transform_mask_into_cache_info(pattern_rba.get_cache_checks_need()));
        }
    }
    else{
        main_automation_checks = transform_mask_into_cache_info(std::vector<bool>(main_automaton.get_size(), true));
        for(const auto& el: pattern_automata)
            pattern_automata_checks.emplace_back(transform_mask_into_cache_info(std::vector<bool>(el.get_size(), true)));
    }
}

cache_checks_container::cache_info cache_checks_container::transform_mask_into_cache_info(const std::vector<bool>& checks_bitmask)const{
    cache_info result{0,std::vector<std::optional<uint>>(checks_bitmask.size(),std::nullopt)};
    for(uint i=0;i<checks_bitmask.size();++i)
        if(checks_bitmask[i])
            result.states_to_cache_correspondence[i] = result.real_size++;
    return result;
}

bool cache_checks_container::should_cache_be_checked(uint state)const{
    return main_automation_checks.states_to_cache_correspondence[state].has_value();
}

bool cache_checks_container::should_cache_be_checked_in_pattern(uint state, uint pattern)const{
    return pattern_automata_checks[pattern].states_to_cache_correspondence[state].has_value();
}

bool cache_checks_container::is_main_cache_needed(void)const{
    return main_automation_checks.real_size > 0;
}

bool cache_checks_container::is_pattern_cache_needed(uint pattern)const{
    return pattern_automata_checks[pattern].real_size > 0;
}

bool cache_checks_container::is_any_cache_needed(void)const{
    if(is_main_cache_needed())
        return true;
    for(uint i=0;i<pattern_automata_checks.size();++i)
        if(is_pattern_cache_needed(i))
            return true;
    return false;
}

void cache_checks_container::generate_resettable_bitarray(cpp_container& output)const{
    output.add_header_include("limits");
    output.add_header_line("template<int states,int cells>");
    output.add_header_line("class resettable_bitarray{");
    output.add_header_line("public:");
    output.add_header_line("inline bool is_set(int state, int cell)const{");
    output.add_header_line("return content[state][cell] == current_threshold;");
    output.add_header_line("}");
    output.add_header_line("");
    output.add_header_line("inline void set(int state, int cell){");
    output.add_header_line("content[state][cell] = current_threshold;");
    output.add_header_line("}");
    output.add_header_line("");
    output.add_header_line("inline void reset(void){");
    output.add_header_line("if(++current_threshold == 0){");
    output.add_header_line("++current_threshold;");
    output.add_header_line("for(unsigned int i=0;i<states;++i){");
    output.add_header_line("for(unsigned int j=0;j<cells;++j){");
    output.add_header_line("content[i][j] = 0;");
    output.add_header_line("}");
    output.add_header_line("}");
    output.add_header_line("}");
    output.add_header_line("}");
    output.add_header_line("");
    output.add_header_line("private:");
    output.add_header_line("unsigned int content[states][cells] = {};");
    output.add_header_line("unsigned int current_threshold = 1;");
    output.add_header_line("};");
    output.add_header_line("");
}

void cache_checks_container::generate_resettable_bitarray_stack(cpp_container& output)const{
    if(is_any_cache_needed()){
        generate_resettable_bitarray(output);
        output.add_header_include("vector");
        output.add_header_line("class resettable_bitarray_stack{");
        output.add_header_line("public:");
        if(is_main_cache_needed()){
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
        }
        for(uint i=0;i<pattern_automata_checks.size();++i)
            if(is_pattern_cache_needed(i)){
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
        if(is_main_cache_needed()){
            output.add_header_line("std::vector<resettable_bitarray<"+std::to_string(main_automation_checks.real_size)+","+std::to_string(board_size)+">> main_content = {};");
            output.add_header_line("unsigned int current_top = 0;");
        }
        for(uint i=0;i<pattern_automata_checks.size();++i)
            if(is_pattern_cache_needed(i)){
                output.add_header_line("std::vector<resettable_bitarray<"+std::to_string(pattern_automata_checks[i].real_size)+","+std::to_string(board_size)+">> pattern_content"+std::to_string(i)+" = {};");
                output.add_header_line("unsigned int pattern_current_top"+std::to_string(i)+" = 0;");
            }
        output.add_header_line("};");
        output.add_header_line("");
    }
    else
        output.add_header_line("class resettable_bitarray_stack{};");
}

uint cache_checks_container::get_cache_cell_to_check(uint state)const{
    assert(should_cache_be_checked(state));
    return *main_automation_checks.states_to_cache_correspondence[state];
}

uint cache_checks_container::get_cache_cell_to_check_in_pattern(uint state, uint pattern)const{
    assert(should_cache_be_checked_in_pattern(state, pattern));
    return *pattern_automata_checks[pattern].states_to_cache_correspondence[state];
}
