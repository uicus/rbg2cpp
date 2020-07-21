#ifndef UNCHECKED_MODIFIERS_COMPILER
#define UNCHECKED_MODIFIERS_COMPILER

#include"abstract_dispatcher.hpp"
#include"types.hpp"
#include<cassert>

struct static_transition_data;
class cpp_container;

namespace rbg_parser{
    class game_move;
    class declarations;
}

class unchecked_modifiers_compiler : public rbg_parser::abstract_dispatcher{
        cpp_container& output;
        const static_transition_data& static_data;
        uint next_state_index;
        bool generate_revert;
    public:
        unchecked_modifiers_compiler(void)=delete;
        unchecked_modifiers_compiler(const unchecked_modifiers_compiler&)=delete;
        unchecked_modifiers_compiler(unchecked_modifiers_compiler&&)=default;
        unchecked_modifiers_compiler& operator=(const unchecked_modifiers_compiler&)=delete;
        unchecked_modifiers_compiler& operator=(unchecked_modifiers_compiler&&)=delete;
        ~unchecked_modifiers_compiler(void)override=default;
        unchecked_modifiers_compiler(cpp_container& output,
                                     const static_transition_data& static_data,
                                     uint next_state_index,
                                     bool generate_revert=false);
        void dispatch(const rbg_parser::sum&)override{assert(false);}
        void dispatch(const rbg_parser::concatenation&)override{assert(false);}
        void dispatch(const rbg_parser::star_move&)override{assert(false);}
        void dispatch(const rbg_parser::shift&)override{};
        void dispatch(const rbg_parser::noop&)override{};
        void dispatch(const rbg_parser::ons&)override{};
        void dispatch(const rbg_parser::off& m)override;
        void dispatch(const rbg_parser::assignment& m)override;
        void dispatch(const rbg_parser::player_switch& m)override;
        void dispatch(const rbg_parser::keeper_switch& m)override;
        void dispatch(const rbg_parser::move_check&)override{}
        void dispatch(const rbg_parser::arithmetic_comparison&)override{};
        void dispatch(const rbg_parser::integer_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::variable_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::arithmetic_operation&)override{assert(false);}
};

#endif
