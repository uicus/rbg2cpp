#include"next_cells_getter.hpp"
#include"shift.hpp"

next_cells_getter::next_cells_getter(uint starting_cell,
                                     const std::vector<std::vector<uint>>& board_structure,
                                     const std::map<rbg_parser::token, uint>& edges_to_id)
  : next_cell(starting_cell)
  , board_structure(board_structure)
  , edges_to_id(edges_to_id){}

std::optional<uint> next_cells_getter::get_next_cell(void)const{
    return next_cell;
}

bool next_cells_getter::was_modifier_encountered(void)const{
    return modifier_encountered;
}

void next_cells_getter::dispatch(const rbg_parser::shift& m){
    if(next_cell){
        uint next_candidate = board_structure[*next_cell][edges_to_id.at(m.get_content())];
        if(next_candidate > 0)
            next_cell = next_candidate-1;
        else
            next_cell = std::nullopt;
    }
}

void next_cells_getter::dispatch(const rbg_parser::off&){
    modifier_encountered = true;
}

void next_cells_getter::dispatch(const rbg_parser::assignment&){
    modifier_encountered = true;
}

void next_cells_getter::dispatch(const rbg_parser::player_switch&){
    modifier_encountered = true;
}

void next_cells_getter::dispatch(const rbg_parser::keeper_switch&){
    modifier_encountered = true;
}
