#include"next_board_getter.hpp"
#include"shift.hpp"
#include"shift_table.hpp"
#include<numeric>
#include<cassert>

next_board_getter::next_board_getter(const std::vector<std::vector<uint>>& board_structure,
                                     const std::map<rbg_parser::token, uint>& edges_to_id)
  : next_cells(board_structure.size())
  , board_structure(board_structure)
  , edges_to_id(edges_to_id){
    std::iota(next_cells.begin(), next_cells.end(), 0);
}

std::optional<std::vector<std::optional<uint>>> next_board_getter::get_next_board(void)const{
    for(uint i=0;i<next_cells.size();++i)
        if(next_cells[i].has_value() and next_cells[i] != i)
            return next_cells;
    return std::nullopt;
}

void next_board_getter::dispatch(const rbg_parser::shift& m){
    for(auto& next_cell: next_cells){
        if(next_cell){
            uint next_candidate = board_structure[*next_cell][edges_to_id.at(m.get_content())];
            if(next_candidate > 0)
                next_cell = next_candidate-1;
            else
                next_cell = std::nullopt;
        }
    }
}

void next_board_getter::apply_shift_table(const shift_table& st){
    assert(not st.can_be_backtraced());
    for(auto& next_cell: next_cells){
        if(next_cell){
            auto candidates = st.get_next_cells(*next_cell);
            assert(candidates.size() <= 1);
            if(candidates.empty() or candidates[0] == 0)
                next_cell = std::nullopt;
            else
                next_cell = candidates[0];
        }
    }
}
