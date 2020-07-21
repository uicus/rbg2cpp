#ifndef MODIFIERS_COUNTER
#define MODIFIERS_COUNTER

#include"abstract_dispatcher.hpp"
#include"types.hpp"
#include<cassert>
#include<vector>

namespace rbg_parser{
    class game_move;
}

class modifiers_counter : public rbg_parser::abstract_dispatcher{
        std::vector<uint> modifiers_count_to_actions_count = {};
    public:
        modifiers_counter(void)=default;
        modifiers_counter(const modifiers_counter&)=delete;
        modifiers_counter(modifiers_counter&&)=default;
        modifiers_counter& operator=(const modifiers_counter&)=delete;
        modifiers_counter& operator=(modifiers_counter&&)=delete;
        ~modifiers_counter(void)override=default;
        std::vector<uint> get_result(void);
        void dispatch(const rbg_parser::sum& m)override;
        void dispatch(const rbg_parser::concatenation& m)override;
        void dispatch(const rbg_parser::star_move& m)override;
        void dispatch(const rbg_parser::shift&)override;
        void dispatch(const rbg_parser::noop&)override;
        void dispatch(const rbg_parser::ons&)override;
        void dispatch(const rbg_parser::off& m)override;
        void dispatch(const rbg_parser::assignment& m)override;
        void dispatch(const rbg_parser::player_switch& m)override;
        void dispatch(const rbg_parser::keeper_switch& m)override;
        void dispatch(const rbg_parser::move_check&)override;
        void dispatch(const rbg_parser::arithmetic_comparison&)override;
        void dispatch(const rbg_parser::integer_arithmetic&)override{assert(false);};
        void dispatch(const rbg_parser::variable_arithmetic&)override{assert(false);};
        void dispatch(const rbg_parser::arithmetic_operation&)override{assert(false);};

};

#endif
