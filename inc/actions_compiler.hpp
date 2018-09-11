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

namespace rbg_parser{
    class game_move;
    class declarations;
}

enum application_type{
    board_change,
    variable_change,
};

struct application{
    application_type type;
    uint additional_info;
};

class actions_compiler : public rbg_parser::abstract_dispatcher{
        cpp_container& output;
        const static_transition_data& static_data;
        std::vector<application> reverting_stack;
        bool encountered_board_change;
        bool encountered_variable_change;
        bool should_check_cell_correctness;
        bool has_modifier;
        bool has_saved_cache_level;
        int next_player;
        bool is_finisher;
        void push_changes_on_board_list(cpp_container& output, const std::string& piece_id);
        void save_board_change_for_later_revert(cpp_container& output, uint piece_id);
        void push_changes_on_variables_list(cpp_container& output, const std::string& variable_id, const std::string& value);
        void save_variable_change_for_later_revert(cpp_container& output, uint variable_id);
        void revert_board_change(cpp_container& output, uint piece_id, uint stack_position)const;
        void revert_variable_change(cpp_container& output, uint variable_id, uint stack_position)const;
    public:
        actions_compiler(void)=delete;
        actions_compiler(const actions_compiler&)=delete;
        actions_compiler(actions_compiler&&)=default;
        actions_compiler& operator=(const actions_compiler&)=delete;
        actions_compiler& operator=(actions_compiler&&)=delete;
        ~actions_compiler(void)override=default;
        actions_compiler(
            cpp_container& output,
            const static_transition_data& static_data);
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
        void insert_reverting_sequence_after_fail(void)const;
        void insert_reverting_sequence_after_success(void)const;
        void insert_unended_reverting_sequence(void)const;
        void finallize(void);
        void check_cell_correctness(void);
        void notify_about_modifier(void);
        void notify_about_cell_change(void);
        bool is_ready_to_report(void)const;
        int get_next_player(void)const;
};

#endif
