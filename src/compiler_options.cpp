#include<cstring>

#include"compiler_options.hpp"
#include"options.hpp"

compiler_options::compiler_options(void):
show_warnings(true),
warnings_as_errors(false),
output_name("game"){
}

compiler_options::compiler_options(uint number_of_args, const char** args):
show_warnings(true),
warnings_as_errors(false),
output_name("game"){
    for(uint i=0;i<number_of_args;++i){
        if(args[i][0] != '-')
            throw rbg_parser::wrong_argument_error("Every flag should begin with \"-\"");
        else{
            if(!std::strcmp(args[i], "-o")){
                ++i;
                if(i >= number_of_args)
                    throw rbg_parser::wrong_argument_error("Flag \"-o\" should be succeeded by output file name");
                else
                    output_name = args[i];
            }
            else if(!std::strcmp(args[i], "-Whide"))
                show_warnings = false;
            else if(!std::strcmp(args[i], "-Werror"))
                warnings_as_errors = true;
            else
                throw rbg_parser::wrong_argument_error("Unrecognized flag");
        }
    }
}

bool compiler_options::showing_warnings(void)const{
    return show_warnings;
}
bool compiler_options::escalating_warnings(void)const{
    return warnings_as_errors;
}

const std::string& compiler_options::output_file(void)const{
    return output_name;
}
