#include"precomputed_pattern.hpp"

void precomputed_pattern::add_cell(uint cell){
    allowed_cells.insert(cell);
}

bool precomputed_pattern::evaluates_to_true(uint cell)const{
    return allowed_cells.count(cell) > 0;
}
