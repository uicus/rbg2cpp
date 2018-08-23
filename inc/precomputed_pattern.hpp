#ifndef PRECOMPUTED_PATTERN
#define PRECOMPUTED_PATTERN

#include<set>

#include"types.hpp"

class precomputed_pattern{
        std::set<uint> allowed_cells;
    public:
        void add_cell(uint cell);
        bool evaluates_to_true(uint cell)const;
        // TODO: printer function
};

#endif
