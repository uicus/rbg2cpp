#ifndef STATE
#define STATE

#include<vector>

#include"types.hpp"
#include"edge.hpp"

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
        void connect_with_state(uint index_in_local_register, const std::vector<const rbg_parser::game_move*>& label_list=std::vector<const rbg_parser::game_move*>(), uint index_after_traversing=0);
};

#endif
