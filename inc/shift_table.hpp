#ifndef SHIFT_TABLE
#define SHIFT_TABLE

#include"types.hpp"

#include<vector>
#include<set>

class precomputed_pattern;
class cpp_container;

class shift_table{
        std::vector<std::set<uint>> next_cells;
        bool there_are_some_choices;
    public:
        shift_table(uint board_size);
        void report_connection(uint source_cell, uint target_cell);
        precomputed_pattern transform_into_pattern(void)const;
        bool can_be_backtraced(void)const;
        void print_array(cpp_container& output, uint index)const;
};

#endif
