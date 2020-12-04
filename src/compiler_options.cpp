#include<cstring>
#include<stdexcept>

#include"compiler_options.hpp"

compiler_options::compiler_options(void):
show_warnings(true),
warnings_as_errors(false),
shift_tables(true),
vectorless_any_squares(true),
safe_monotonicity_methods(false),
cache_optimisation(true),
generate_moves_getter(true),
generate_actions_getter(false),
generate_dotsplit_getter(false),
output_name("game"){
}

compiler_options::compiler_options(uint number_of_args, const char** args):
show_warnings(true),
warnings_as_errors(false),
shift_tables(true),
vectorless_any_squares(true),
safe_monotonicity_methods(false),
cache_optimisation(true),
generate_moves_getter(true),
generate_actions_getter(false),
generate_dotsplit_getter(false),
output_name("game"){
    for(uint i=0;i<number_of_args;++i){
        if(args[i][0] != '-')
            throw std::invalid_argument("Every flag should begin with \"-\"");
        else{
            if(!std::strcmp(args[i], "-o")){
                ++i;
                if(i >= number_of_args)
                    throw std::invalid_argument("Flag \"-o\" should be succeeded by output file name");
                else
                    output_name = args[i];
            }
            else if(!std::strcmp(args[i], "-Whide"))
                show_warnings = false;
            else if(!std::strcmp(args[i], "-Werror"))
                warnings_as_errors = true;
            else if(!std::strcmp(args[i], "-fno-shift-tables"))
                shift_tables = false;
            else if(!std::strcmp(args[i], "-fno-anysquare-opt"))
                vectorless_any_squares = false;
            else if(!std::strcmp(args[i], "-fmonotonic-moves-safe"))
                safe_monotonicity_methods = true;
            else if(!std::strcmp(args[i], "-fno-cache-opt"))
                cache_optimisation = false;
            else if(!std::strcmp(args[i], "-getters")) {
                ++i;
                if (i >= number_of_args)
                    throw std::invalid_argument("Flag \"-getters\" should be succeeded by a string of getters ids");
                std::string arg(args[i]);
                generate_moves_getter = (arg.find('m') != std::string::npos);
                generate_actions_getter = (arg.find('a') != std::string::npos);
                generate_dotsplit_getter = (arg.find('s') != std::string::npos);
            }
            else
                throw std::invalid_argument("Unrecognized flag: "+std::string(args[i]));
        }
    }
}

bool compiler_options::showing_warnings(void)const{
    return show_warnings;
}
bool compiler_options::escalating_warnings(void)const{
    return warnings_as_errors;
}

bool compiler_options::enabled_shift_tables(void)const{
    return shift_tables;
}

bool compiler_options::enabled_any_square_optimisation(void)const{
    return vectorless_any_squares;
}

bool compiler_options::enabled_safe_monotonicity_methods(void)const{
    return safe_monotonicity_methods;
}

bool compiler_options::enabled_cache_optimisation(void)const{
    return cache_optimisation;
}

bool compiler_options::enabled_moves_getter(void)const{
    return generate_moves_getter;
}

bool compiler_options::enabled_actions_getter(void)const{
    return generate_actions_getter;
}

bool compiler_options::enabled_dotsplit_getter(void)const{
    return generate_dotsplit_getter;
}

bool compiler_options::enabled_semisplit(void)const{
    return generate_actions_getter || generate_dotsplit_getter;
}

const std::string& compiler_options::output_file(void)const{
    return output_name;
}
