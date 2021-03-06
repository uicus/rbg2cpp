#include"precomputed_pattern.hpp"
#include"cpp_container.hpp"
#include"transition_data.hpp"

precomputed_pattern::precomputed_pattern(uint overall_size):
allowed_cells(),
overall_size(overall_size){
}

void precomputed_pattern::add_cell(uint cell){
    allowed_cells.insert(cell);
}

bool precomputed_pattern::evaluates_to_true(uint cell)const{
    return allowed_cells.count(cell) > 0;
}

void precomputed_pattern::negate(void){
    std::set<uint> new_allowed;
    for(uint i=0;i<overall_size;++i)
        if(not evaluates_to_true(i))
            new_allowed.insert(i);
    std::swap(new_allowed, allowed_cells);
}

void precomputed_pattern::print_inside_transition(cpp_container& output, const dynamic_transition_data& dynamic_data)const{
    if(allowed_cells.size()<overall_size/2+1){
        output.add_source_line("switch(cell){");
        for(const auto& el: allowed_cells)
            output.add_source_line("case "+std::to_string(el+1)+":");
        output.add_source_line("break;");
        output.add_source_line("default:");
        dynamic_data.insert_reverting_sequence_after_fail(output);
        output.add_source_line("}");
    }
    else{
        output.add_source_line("switch(cell){");
        for(uint i=0;i<overall_size;++i)
            if(not allowed_cells.count(i))
                output.add_source_line("case "+std::to_string(i+1)+":");
        dynamic_data.insert_reverting_sequence_after_fail(output);
        output.add_source_line("default:");
        output.add_source_line("break;");
        output.add_source_line("}");
    }
}
