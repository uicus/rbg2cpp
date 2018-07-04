#ifndef STATE
#define STATE

#include<vector>
#include<set>

#include"types.hpp"
#include"edge.hpp"

namespace rbg_parser{
    class game_move;
}

struct move_check_endpoints_info{
    uint start_state;
    uint accept_state;
    bool negated;
    bool operator==(const move_check_endpoints_info& rhs){
        return start_state == rhs.start_state
           and accept_state == rhs.accept_state
           and negated == rhs.negated;
    }
    bool operator<(const move_check_endpoints_info& rhs)const{
        return start_state < rhs.start_state
            or (start_state == rhs.start_state and accept_state < rhs.accept_state)
            or (start_state == rhs.start_state and accept_state == rhs.accept_state and negated < rhs.negated);
    }
};

class state{
        uint state_id;
        static uint next_free_id;
        std::vector<edge> next_states;
        std::vector<move_check_endpoints_info> started_checks;
        std::set<move_check_endpoints_info> ended_checks;
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
        void set_as_check_begin(uint check_start, uint check_end, bool negated);
        void set_as_check_end(uint check_start, uint check_end, bool negated);
};

#endif
