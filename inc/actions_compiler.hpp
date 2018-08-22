#ifndef ACTIONS_COMPILER
#define ACTIONS_COMPILER

#include"abstract_dispatcher.hpp"
#include"types.hpp"
#include"token.hpp"
#include<cassert>
#include<map>
#include<string>

class cpp_container;

namespace rbg_parser{
    class game_move;
    class declarations;
}

class actions_compiler : public rbg_parser::abstract_dispatcher{
        cpp_container& output;
        const std::map<rbg_parser::token, uint>& pieces_to_id;
        const std::map<rbg_parser::token, uint>& edges_to_id;
        const std::map<rbg_parser::token, uint>& variables_to_id;
        const rbg_parser::declarations& decl;
        const std::string& reverting_function;
        const std::string& cache_pusher;
        bool should_check_cell_correctness;
        bool has_modifier;
        bool is_finisher;
        bool should_build_move;
    public:
        actions_compiler(void)=delete;
        actions_compiler(const actions_compiler&)=delete;
        actions_compiler(actions_compiler&&)=default;
        actions_compiler& operator=(const actions_compiler&)=delete;
        actions_compiler& operator=(actions_compiler&&)=delete;
        ~actions_compiler(void)override=default;
        actions_compiler(
            cpp_container& output,
            const std::map<rbg_parser::token, uint>& pieces_to_id,
            const std::map<rbg_parser::token, uint>& edges_to_id,
            const std::map<rbg_parser::token, uint>& variables_to_id,
            const rbg_parser::declarations& decl,
            const std::string& reverting_function,
            const std::string& cache_pusher,
            bool should_build_move);
        void dispatch(const rbg_parser::sum&)override{assert(false);}
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
        void finallize(void);
        void check_cell_correctness(void);
        void notify_about_modifier(void);
        bool is_ready_to_report(void)const;
};

#endif
