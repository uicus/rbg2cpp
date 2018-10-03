#ifndef ACTIONS_COMPILER
#define ACTIONS_COMPILER

#include"abstract_dispatcher.hpp"
#include"types.hpp"
#include"token.hpp"
#include<cassert>
#include<map>
#include<string>

class cpp_container;
struct static_transition_data;
class dynamic_transition_data;

namespace rbg_parser{
    class game_move;
    class declarations;
}

class actions_compiler : public rbg_parser::abstract_dispatcher{
        cpp_container& output;
        const static_transition_data& static_data;
        dynamic_transition_data& dynamic_data;
        void print_variable_assignment(uint variable_id, const std::string& rvalue);
    public:
        actions_compiler(void)=delete;
        actions_compiler(const actions_compiler&)=delete;
        actions_compiler(actions_compiler&&)=default;
        actions_compiler& operator=(const actions_compiler&)=delete;
        actions_compiler& operator=(actions_compiler&&)=delete;
        ~actions_compiler(void)override=default;
        actions_compiler(
            cpp_container& output,
            const static_transition_data& static_data,
            dynamic_transition_data& dynamic_data);
        void dispatch(const rbg_parser::sum&)override{assert(false);}
        void dispatch(const rbg_parser::prioritized_sum&)override{assert(false);}
        void dispatch(const rbg_parser::concatenation&)override{assert(false);}
        void dispatch(const rbg_parser::star_move&)override{assert(false);}
        void dispatch(const rbg_parser::shift& m)override;
        void dispatch(const rbg_parser::ons& m)override;
        void dispatch(const rbg_parser::off& m)override;
        void dispatch(const rbg_parser::assignment& m)override;
        void dispatch(const rbg_parser::player_switch& m)override;
        void dispatch(const rbg_parser::keeper_switch&)override;
        void dispatch(const rbg_parser::move_check&)override{assert(false);}
        void dispatch(const rbg_parser::arithmetic_comparison& m)override;
        void dispatch(const rbg_parser::integer_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::variable_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::arithmetic_operation&)override{assert(false);}
};

#endif
