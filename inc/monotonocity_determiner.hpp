#ifndef MONOTONICITY_DETERMINER
#define MONOTONICITY_DETERMINER

#include"abstract_dispatcher.hpp"
#include"token.hpp"
#include<cassert>
#include<set>

namespace rbg_parser{
    class game_move;
}

class monotonicity_determiner : public rbg_parser::abstract_dispatcher{
        std::set<rbg_parser::token> all_ons_in_monotonics = {};
        std::set<rbg_parser::token> ons_in_current_monotonic = {};
        std::set<rbg_parser::token> all_used_offs = {};
    public:
        monotonicity_determiner(void)=default;
        monotonicity_determiner(const monotonicity_determiner&)=delete;
        monotonicity_determiner(monotonicity_determiner&&)=default;
        monotonicity_determiner& operator=(const monotonicity_determiner&)=delete;
        monotonicity_determiner& operator=(monotonicity_determiner&&)=delete;
        ~monotonicity_determiner(void)override=default;
        void dispatch(const rbg_parser::sum&)override{assert(false);};
        void dispatch(const rbg_parser::prioritized_sum&)override{assert(false);};
        void dispatch(const rbg_parser::concatenation&)override{assert(false);};
        void dispatch(const rbg_parser::star_move&)override{assert(false);};
        void dispatch(const rbg_parser::shift&)override;
        void dispatch(const rbg_parser::ons&)override;
        void dispatch(const rbg_parser::off& m)override;
        void dispatch(const rbg_parser::assignment& m)override;
        void dispatch(const rbg_parser::player_switch& m)override;
        void dispatch(const rbg_parser::keeper_switch& m)override;
        void dispatch(const rbg_parser::move_check& m)override;
        void dispatch(const rbg_parser::arithmetic_comparison& m)override;
        void dispatch(const rbg_parser::integer_arithmetic&)override{assert(false);};
        void dispatch(const rbg_parser::variable_arithmetic&)override{assert(false);};
        void dispatch(const rbg_parser::arithmetic_operation&)override{assert(false);};
        std::set<rbg_parser::token> get_ons_from_monotonics(void)const;
        std::set<rbg_parser::token> get_all_offs(void)const;
        void notify_about_monotonic_end(void);
};

#endif
