#include"monotonic_move.hpp"
#include"cpp_container.hpp"
#include"shift_table.hpp"
#include"transition_data.hpp"

void monotonic_move::print_legality_check(cpp_container& output, const static_transition_data& static_data)const{
    cell_choice->print_monotonic_checker(output);
    if(pieces_choice){
        output.add_source_line("switch(pieces[m.mr.front().cell]){");
        for(const auto& el: *pieces_choice)
            output.add_source_line("case "+std::to_string(static_data.pieces_to_id.at(el))+":");
        output.add_source_line("break;");
        output.add_source_line("default:");
        output.add_source_line("return false;");
        output.add_source_line("}");
    }
    output.add_source_line("switch(m.mr.front().index){");
    for(const auto el: end_action_indices)
        output.add_source_line("case "+std::to_string(el)+":");
    output.add_source_line("break;");
    output.add_source_line("default:");
    output.add_source_line("return false;");
    output.add_source_line("}");
}
