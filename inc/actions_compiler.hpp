#ifndef ACTIONS_COMPILER
#define ACTIONS_COMPILER

#include"abstract_dispatcher.hpp"
#include"types.hpp"
#include<cassert>

namespace rbg_parser{
    class game_move;
}

class actions_compiler : public rbg_parser::abstract_dispatcher{
    public:
        actions_compiler(void)=default;
        actions_compiler(const actions_compiler&)=delete;
        actions_compiler(actions_compiler&&)=default;
        actions_compiler& operator=(const actions_compiler&)=delete;
        actions_compiler& operator=(actions_compiler&&)=default;
        ~actions_compiler(void)override=default;
        void dispatch(const rbg_parser::sum& m)override;
        void dispatch(const rbg_parser::concatenation& m)override;
        void dispatch(const rbg_parser::star_move& m)override;
        void dispatch(const rbg_parser::shift& m)override;
        void dispatch(const rbg_parser::ons& m)override;
        void dispatch(const rbg_parser::off& m)override;
        void dispatch(const rbg_parser::assignment& m)override;
        void dispatch(const rbg_parser::player_switch& m)override;
        void dispatch(const rbg_parser::keeper_switch& m)override;
        void dispatch(const rbg_parser::move_check& m)override;
        void dispatch(const rbg_parser::actions_block& m)override;
        void dispatch(const rbg_parser::arithmetic_comparison& m)override;
        void dispatch(const rbg_parser::integer_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::variable_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::arithmetic_operation&)override{assert(false);}
};

#endif
