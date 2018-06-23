#include<iostream>
#include"token.hpp"
#include"message.hpp"

int main(){
    std::cout<<"Initial commit"<<std::endl;
    rbg_parser::messages_container msg;
    std::vector<rbg_parser::token> result = rbg_parser::tokenize("#pieces=",msg);
    return 0;
}
