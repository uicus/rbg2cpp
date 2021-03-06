#ifndef COMPILER_OPTIONS
#define COMPILER_OPTIONS

#include<string>
#include<exception>

#include"types.hpp"

class compiler_options{
        bool show_warnings : 1;
        bool warnings_as_errors : 1;
        bool shift_tables : 1;
        bool vectorless_any_squares : 1;
        bool safe_monotonicity_methods : 1;
        bool cache_optimisation : 1;
        bool generate_moves_getter : 1;
        bool generate_actions_getter : 1;
        bool generate_dotsplit_getter : 1;
        std::string output_name;
    public:
        compiler_options(void);
        compiler_options(uint number_of_args, const char** args);

        bool showing_warnings(void)const;
        bool escalating_warnings(void)const;
        bool enabled_shift_tables(void)const;
        bool enabled_any_square_optimisation(void)const;
        bool enabled_safe_monotonicity_methods(void)const;
        bool enabled_cache_optimisation(void)const;
        bool enabled_moves_getter(void)const;
        bool enabled_actions_getter(void)const;
        bool enabled_dotsplit_getter(void)const;
        bool enabled_semisplit(void)const;
        const std::string& output_file(void)const;
};

#endif
