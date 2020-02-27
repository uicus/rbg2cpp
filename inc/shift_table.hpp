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
        bool any_square;
    public:
        bool operator==(const shift_table& rhs)const;
        shift_table(uint board_size);
        uint get_size(void)const;
        void report_connection(uint source_cell, uint target_cell);
        precomputed_pattern transform_into_pattern(void)const;
        bool can_be_backtraced(void)const;
        void print_array(cpp_container& output, uint index)const;
        bool is_any_square(void)const;
        void check_if_any_square(void);
        std::vector<uint> get_next_cells(uint starting_cell)const;
        bool are_all_the_same(void)const;
        void print_monotonic_checker(cpp_container& output)const;
};

uint insert_shift_table(std::vector<shift_table>& shift_tables, shift_table&& st);

#endif
