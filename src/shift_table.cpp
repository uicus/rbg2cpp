#include"shift_table.hpp"
#include"precomputed_pattern.hpp"
#include"cpp_container.hpp"
#include<cassert>

shift_table::shift_table(uint board_size):
next_cells(board_size),
there_are_some_choices(false),
any_square(false){
}

uint shift_table::get_size(void)const{
    return next_cells.size();
}

bool shift_table::operator==(const shift_table& rhs)const{
    assert(rhs.next_cells.size() == next_cells.size());
    for(uint i=0;i<next_cells.size();++i){
        if(next_cells[i] != rhs.next_cells[i])
            return false;
    }
    return true;
}

void shift_table::report_connection(uint source_cell, uint target_cell){
    next_cells[source_cell].insert(target_cell);
    if(next_cells[source_cell].size()>1)
        there_are_some_choices = true;
}

precomputed_pattern shift_table::transform_into_pattern(void)const{
    precomputed_pattern result(next_cells.size());
    for(uint i=0;i<next_cells.size();++i)
        if(not next_cells[i].empty())
            result.add_cell(i);
    return result;
}

bool shift_table::can_be_backtraced(void)const{
    return there_are_some_choices;
}

void shift_table::print_array(cpp_container& output, uint index)const{
    if(not any_square){
        if(there_are_some_choices){
            output.add_source_include("vector");
            output.add_source_line("static const std::vector<int> shift_table"+std::to_string(index)+"["+std::to_string(next_cells.size()+1)+"] = {");
            output.add_source_line("{},");
            for(const auto& el: next_cells){
                std::string cell_line = "{";
                for(const auto& set_el: el)
                    cell_line += std::to_string(set_el+1)+",";
                cell_line += "},";
                output.add_source_line(cell_line);
            }
            output.add_source_line("};");
            output.add_source_line("");
        }
        else{
            output.add_source_line("static const int shift_table"+std::to_string(index)+"["+std::to_string(next_cells.size()+1)+"] = {");
            std::string cell_line = "0,";
            for(const auto& el: next_cells){
                assert(el.size()<=1);
                if(el.empty())
                    cell_line += "0,";
                else
                    cell_line += std::to_string((*el.begin())+1)+",";
            }
            output.add_source_line(cell_line);
            output.add_source_line("};");
            output.add_source_line("");
        }
    }
}

void shift_table::print_monotonic_checker(cpp_container& output)const{
    if(not any_square){
        const std::set<uint>& allowed_cells = next_cells.front();
        uint overall_size = next_cells.size();
        if(allowed_cells.size()<overall_size/2+1){
            output.add_source_line("switch(m.mr.front().cell){");
            for(const auto& el: allowed_cells)
                output.add_source_line("case "+std::to_string(el+1)+":");
            output.add_source_line("break;");
            output.add_source_line("default:");
            output.add_source_line("return false;");
            output.add_source_line("}");
        }
        else{
            output.add_source_line("switch(m.mr.front().cell){");
            for(uint i=0;i<overall_size;++i)
                if(not allowed_cells.count(i))
                    output.add_source_line("case "+std::to_string(i+1)+":");
            output.add_source_line("return false;");
            output.add_source_line("default:");
            output.add_source_line("break;");
            output.add_source_line("}");
        }
    }
}

bool shift_table::is_any_square(void)const{
    return any_square;
}

void shift_table::check_if_any_square(void){
    for(const auto& el: next_cells)
        if(el.size() < next_cells.size())
            return;
    any_square = true;
}

std::vector<uint> shift_table::get_next_cells(uint starting_cell)const{
    return std::vector<uint>(next_cells[starting_cell].begin(), next_cells[starting_cell].end());
}

uint insert_shift_table(std::vector<shift_table>& shift_tables, shift_table&& st){
    for(uint i=0;i<shift_tables.size();++i)
        if(shift_tables[i]==st)
            return i;
    shift_tables.push_back(std::move(st));
    return shift_tables.size()-1;
}

bool shift_table::are_all_the_same(void)const{
    for(const auto& el: next_cells)
        if(el != next_cells[0])
            return false;
    return true;
}
