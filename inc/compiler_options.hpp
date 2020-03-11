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
        bool semi_split_generation : 1;
        bool safe_monotonicity_methods : 1;
        bool cache_optimisation : 1;
        std::string output_name;
    public:
        compiler_options(void);
        compiler_options(uint number_of_args, const char** args);

        bool showing_warnings(void)const;
        bool escalating_warnings(void)const;
        bool enabled_shift_tables(void)const;
        bool enabled_any_square_optimisation(void)const;
        bool enabled_semi_split_generation(void)const;
        bool enabled_safe_monotonicity_methods(void)const;
        bool enabled_cache_optimisation(void)const;
        const std::string& output_file(void)const;
};

#endif
