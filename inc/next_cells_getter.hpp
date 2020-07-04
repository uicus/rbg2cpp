#ifndef NEXT_CELLS_GETTER
#define NEXT_CELLS_GETTER

#include"abstract_dispatcher.hpp"
#include"types.hpp"
#include<cassert>
#include<vector>
#include<map>
#include<optional>

namespace rbg_parser{
    class token;
}

class next_cells_getter : public rbg_parser::abstract_dispatcher{
        bool modifier_encountered = false;
        std::optional<uint> next_cell;
        const std::vector<std::vector<uint>>& board_structure;
        const std::map<rbg_parser::token, uint>& edges_to_id;
    public:
        next_cells_getter(void)=delete;
        next_cells_getter(const next_cells_getter&)=delete;
        next_cells_getter(next_cells_getter&&)=default;
        next_cells_getter& operator=(const next_cells_getter&)=delete;
        next_cells_getter& operator=(next_cells_getter&&)=delete;
        ~next_cells_getter(void)override=default;
        next_cells_getter(uint starting_cell,
                          const std::vector<std::vector<uint>>& board_structure,
                          const std::map<rbg_parser::token, uint>& edges_to_id);
        void dispatch(const rbg_parser::sum&)override{assert(false);}
        void dispatch(const rbg_parser::prioritized_sum&)override{assert(false);}
        void dispatch(const rbg_parser::concatenation&)override{assert(false);}
        void dispatch(const rbg_parser::star_move&)override{assert(false);}
        void dispatch(const rbg_parser::shift& m)override;
        void dispatch(const rbg_parser::noop&)override{};
        void dispatch(const rbg_parser::ons&)override{}
        void dispatch(const rbg_parser::off&)override;
        void dispatch(const rbg_parser::assignment&)override;
        void dispatch(const rbg_parser::player_switch&)override;
        void dispatch(const rbg_parser::keeper_switch&)override;
        void dispatch(const rbg_parser::move_check&)override{}
        void dispatch(const rbg_parser::arithmetic_comparison&)override{}
        void dispatch(const rbg_parser::integer_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::variable_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::arithmetic_operation&)override{assert(false);}
        std::optional<uint> get_next_cell(void)const;
        bool was_modifier_encountered(void)const;
};



#endif
