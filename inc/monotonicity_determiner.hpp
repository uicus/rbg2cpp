#ifndef MONOTONICITY_DETERMINER
#define MONOTONICITY_DETERMINER

#include"abstract_dispatcher.hpp"
#include"token.hpp"
#include<cassert>
#include<set>
#include<optional>

namespace rbg_parser{
    class game_move;
}
class shift_table;

struct monotonic_move{
    uint start_state;
    const shift_table* cell_choice;
    std::optional<std::set<rbg_parser::token>> pieces_choice;
};

class monotonicity_determiner : public rbg_parser::abstract_dispatcher{
        std::set<rbg_parser::token> all_used_offs = {};
        std::vector<monotonic_move> monotonics = {};
        monotonic_move current_move = {};
        enum {
            beginning,
            after_initial_switch,
            after_shift_table,
            only_finishing_switch_acceptable,
            after_first_finishing_switch,
            ruined,
        } current_state = beginning;
        uint automaton_state = 0;
        void handle_non_monotonic_action(void);
        bool is_monotonic_ruined_by_off(const monotonic_move& m)const;
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
        void dispatch(const rbg_parser::ons& m)override;
        void dispatch(const rbg_parser::off& m)override;
        void dispatch(const rbg_parser::assignment&)override;
        void dispatch(const rbg_parser::player_switch& m)override;
        void dispatch(const rbg_parser::keeper_switch& m)override;
        void dispatch(const rbg_parser::move_check&)override;
        void dispatch(const rbg_parser::arithmetic_comparison&)override;
        void dispatch(const rbg_parser::integer_arithmetic&)override{assert(false);};
        void dispatch(const rbg_parser::variable_arithmetic&)override{assert(false);};
        void dispatch(const rbg_parser::arithmetic_operation&)override{assert(false);};
        void dispatch_shift_table(const shift_table& table);
        std::set<rbg_parser::token> get_all_offs(void)const;
        void notify_about_last_alternative(void);
        void notify_about_alternative_start(void);
        void notify_about_automaton_state(uint state);
        std::vector<monotonic_move> get_final_result(void)const;
};

#endif
