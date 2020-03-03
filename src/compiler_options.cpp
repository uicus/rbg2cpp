#include<cstring>
#include<stdexcept>

#include"compiler_options.hpp"

compiler_options::compiler_options(void):
show_warnings(true),
warnings_as_errors(false),
shift_tables(true),
vectorless_any_squares(true),
semi_split_generation(false),
safe_monotonicity_methods(false),
output_name("game"){
}

compiler_options::compiler_options(uint number_of_args, const char** args):
show_warnings(true),
warnings_as_errors(false),
shift_tables(true),
vectorless_any_squares(true),
semi_split_generation(false),
safe_monotonicity_methods(false),
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
            else if(!std::strcmp(args[i], "-fsemi-split"))
                semi_split_generation = true;
            else if(!std::strcmp(args[i], "-fmonotonic-moves-safe"))
                safe_monotonicity_methods = true;
            else
                throw std::invalid_argument("Unrecognized flag");
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

bool compiler_options::enabled_semi_split_generation(void)const{
    return semi_split_generation;
}

bool compiler_options::enabled_safe_monotonicity_methods(void)const{
    return safe_monotonicity_methods;
}

const std::string& compiler_options::output_file(void)const{
    return output_name;
}
