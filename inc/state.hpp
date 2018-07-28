#ifndef STATE
#define STATE

#include<vector>
#include<set>

#include"types.hpp"
#include"edge.hpp"

class cpp_container;

namespace rbg_parser{
    class game_move;
}

class state{
        uint state_id;
        static uint next_free_id;
        std::vector<edge> next_states;
    public:
        state(void);
        state(const state& rhs);
        state(state&&)=default;
        state& operator=(const state&);
        state& operator=(state&&)=default;
        ~state(void)=default;
        uint get_id(void)const;
        void inform_about_being_appended(uint shift_value);
        void inform_about_state_deletion(uint deleted_index);
        void absorb(state&& rhs);
        void connect_with_state(uint index_in_local_register, const std::vector<label>& label_list=std::vector<label>(), uint index_after_traversing=0);
        void print_transition_functions(
            uint from_state,
            cpp_container& output,
            const std::map<rbg_parser::token, uint>& pieces_to_id,
            const std::map<rbg_parser::token, uint>& edges_to_id,
            const std::map<rbg_parser::token, uint>& variables_to_id,
            const rbg_parser::declarations& decl,
            const std::vector<state>& local_register)const;
        void print_outgoing_transitions(uint from_state, cpp_container& output)const;
        bool is_dead_end(void)const;
        bool is_no_choicer(void)const;
};

#endif
