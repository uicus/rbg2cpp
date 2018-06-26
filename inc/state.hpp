#ifndef STATE
#define STATE

#include<vector>

#include"types.hpp"

class edge;

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
};

#endif
