#ifndef AUTOMATON
#define AUTOMATON

#include<vector>

#include"types.hpp"

namespace rbg_parser{
    class game_move;
}

class state;

class automaton{
        std::vector<state> local_register;
        uint start_state;
        uint accept_state;
        std::pair<uint,uint> place_side_by_side(automaton&& rhs);
        std::pair<uint,uint> prepare_new_endpoints(void);
        void set_endpoints(const std::pair<uint,uint>& new_endpoints);
        void concat_automaton(automaton&& concatee);
    public:
        void starify_automaton(void);
        void repeat_automaton(uint times);
        uint get_start_state(void);
        friend automaton sum_of_automatons(std::vector<automaton>&& elements);
        friend automaton concatenation_of_automatons(std::vector<automaton>&& elements);
        friend automaton edge_automaton(const std::vector<const rbg_parser::game_move*>& label_list);
};

automaton sum_of_automatons(std::vector<automaton>&& elements);
automaton concatenation_of_automatons(std::vector<automaton>&& elements);
automaton edge_automaton(const std::vector<const rbg_parser::game_move*>& label_list);

#endif
