#ifndef AUTOMATON_BUILDER
#define AUTOMATON_BUILDER

#include"abstract_dispatcher.hpp"
#include"automaton.hpp"
#include"types.hpp"
#include<cassert>
#include<vector>

namespace rbg_parser{
    class game_move;
}

class automaton_builder : public rbg_parser::abstract_dispatcher{
        automaton local_copy_automaton;
        automaton& currently_modified_automaton;
        bool has_automaton;
        std::vector<automaton>& pattern_automata;
        std::vector<label>& current_block;
        void handle_any_switch(const rbg_parser::game_move& m);
        void build_automaton_from_actions_so_far();
        void concat_automaton_to_result_so_far(automaton&& a);
        automaton_builder(std::vector<automaton>& pattern_automata, std::vector<label>& current_block, automaton& upper_level_automaton);
        automaton_builder delegate_builder();
    public:
        automaton_builder(const automaton_builder&)=delete;
        automaton_builder(automaton_builder&&)=default;
        automaton_builder& operator=(const automaton_builder&)=delete;
        automaton_builder& operator=(automaton_builder&&)=delete;
        ~automaton_builder(void)override=default;
        automaton_builder(std::vector<automaton>& pattern_automata, std::vector<label>& current_block);
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
        void dispatch(const rbg_parser::arithmetic_comparison& m)override;
        void dispatch(const rbg_parser::integer_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::variable_arithmetic&)override{assert(false);}
        void dispatch(const rbg_parser::arithmetic_operation&)override{assert(false);}
        automaton get_final_result(void);
};

#endif
