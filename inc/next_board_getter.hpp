#ifndef NEXT_BOARD_GETTER
#define NEXT_BOARD_GETTER

#include"abstract_dispatcher.hpp"
#include"types.hpp"
#include<cassert>
#include<vector>
#include<map>
#include<optional>

namespace rbg_parser{
    class token;
}

class shift_table;

class next_board_getter : public rbg_parser::abstract_dispatcher{
        std::vector<std::optional<uint>> next_cells;
        const std::vector<std::vector<uint>>& board_structure;
        const std::map<rbg_parser::token, uint>& edges_to_id;
    public:
        next_board_getter(void)=delete;
        next_board_getter(const next_board_getter&)=delete;
        next_board_getter(next_board_getter&&)=default;
        next_board_getter& operator=(const next_board_getter&)=delete;
        next_board_getter& operator=(next_board_getter&&)=delete;
        ~next_board_getter(void)override=default;
        next_board_getter(const std::vector<std::vector<uint>>& board_structure,
                          const std::map<rbg_parser::token, uint>& edges_to_id);
        void dispatch(const rbg_parser::sum&)override{assert(false);}
        void dispatch(const rbg_parser::prioritized_sum&)override{assert(false);}
        void dispatch(const rbg_parser::concatenation&)override{assert(false);}
        void dispatch(const rbg_parser::star_move&)override{assert(false);}
        void dispatch(const rbg_parser::shift& m)override;
        void dispatch(const rbg_parser::ons&)override{}
        void dispatch(const rbg_parser::off&)override{}
        void dispatch(const rbg_parser::assignment&)override{}
        void dispatch(const rbg_parser::player_switch&)override{}
        void dispatch(const rbg_parser::keeper_switch&)override{}
        void dispatch(const rbg_parser::move_check&)override{}
        void dispatch(const rbg_parser::arithmetic_comparison&)override{}
        void dispatch(const rbg_parser::integer_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::variable_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::arithmetic_operation&)override{assert(false);}
        void apply_shift_table(const shift_table& st);
        std::optional<std::vector<std::optional<uint>>> get_next_board(void)const;
};



#endif
