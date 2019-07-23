#include"resettable_bitarray.hpp"
#include<algorithm>

resettable_bitarray::resettable_bitarray(uint width, uint height)
  : content(width*height)
  , height(height){}

void resettable_bitarray::reset(void){
    if(++threshold == 0){
        ++threshold;
        std::fill(content.begin(), content.end(), 0);
    }
}

bool resettable_bitarray::is_set(uint x, uint y)const{
    return content[x*height+y] >= threshold;
}

void resettable_bitarray::set(uint x, uint y){
    content[x*height+y] = threshold;
}
