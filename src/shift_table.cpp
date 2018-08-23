#include"shift_table.hpp"
#include"precomputed_pattern.hpp"

shift_table::shift_table(uint board_size):
next_cells(board_size),
there_are_some_choices(false){
}

void shift_table::report_connection(uint source_cell, uint target_cell){
    next_cells[source_cell].insert(target_cell);
    if(next_cells[source_cell].size()>1)
        there_are_some_choices = true;
}

precomputed_pattern shift_table::transform_into_pattern(void)const{
    precomputed_pattern result;
    for(uint i=0;i<next_cells.size();++i)
        if(not next_cells[i].empty())
            result.add_cell(i);
    return result;
}

bool shift_table::can_be_backtraced(void)const{
    return there_are_some_choices;
}
