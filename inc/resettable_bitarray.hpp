#ifndef RESETTABLE_BITARRAY
#define RESETTABLE_BITARRAY

#include"types.hpp"

#include<vector>

class resettable_bitarray{
        std::vector<uint> content;
        uint threshold = 0;
        uint height;
    public:
        resettable_bitarray(void)=delete;
        resettable_bitarray(const resettable_bitarray&)=delete;
        resettable_bitarray& operator=(const resettable_bitarray&)=delete;
        resettable_bitarray(resettable_bitarray&&)=default;
        resettable_bitarray& operator=(resettable_bitarray&&)=default;
        ~resettable_bitarray(void)=default;
        resettable_bitarray(uint width, uint height);
        void reset(void);
        bool is_set(uint x, uint y)const;
        void set(uint x, uint y);
};

#endif
