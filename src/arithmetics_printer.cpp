#include"arithmetics_printer.hpp"
#include"arithmetic_expression.hpp"
#include"integer_arithmetic.hpp"
#include"variable_arithmetic.hpp"
#include"arithmetic_operation.hpp"

arithmetics_printer::arithmetics_printer(const std::map<rbg_parser::token, uint>& pieces_to_id, const std::map<rbg_parser::token, uint>& variables_to_id):
pieces_to_id(pieces_to_id),
variables_to_id(variables_to_id),
final_result(){
}

void arithmetics_printer::dispatch(const rbg_parser::integer_arithmetic& m){
    final_result = std::to_string(m.get_content());
}

void arithmetics_printer::dispatch(const rbg_parser::variable_arithmetic& m){
    const auto& var = m.get_content();
    auto it = pieces_to_id.find(var);
    if(it != pieces_to_id.end())
        final_result = "state_to_change.pieces_count["+std::to_string(it->second)+"]";
    else{
        auto it = variables_to_id.find(var);
        assert(it != variables_to_id.end());
        final_result = "state_to_change.variables["+std::to_string(it->second)+"]";
    }
}

void arithmetics_printer::dispatch(const rbg_parser::arithmetic_operation& m){
    const auto& elements = m.get_content();
    char operation_sign = '\0';
    switch(m.get_operation()){
        case rbg_parser::addition:
            operation_sign = '+';
            break;
        case rbg_parser::subtraction:
            operation_sign = '-';
            break;
        case rbg_parser::multiplication:
            operation_sign = '*';
            break;
        case rbg_parser::division:
            operation_sign = '/';
            break;
        default:
            assert(false);
    }
    for(uint i=0;i<elements.size();++i){
        arithmetics_printer element_printer(pieces_to_id, variables_to_id);
        elements[i]->accept(element_printer);
        final_result += "("+element_printer.get_final_result()+")";
        if(i!=elements.size()-1)
            final_result += operation_sign;
    }
}

std::string arithmetics_printer::get_final_result(void){
    return std::move(final_result);
}
