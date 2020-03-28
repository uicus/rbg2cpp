#ifndef _RBG_RANDOM_GENERATOR_HPP
#define _RBG_RANDOM_GENERATOR_HPP

#if RBG_RANDOM_GENERATOR == 1

// Exact reimplementation of the standard Java generator
#include <cstdint>
struct RBGRandomGenerator {
  uint64_t seed;

  RBGRandomGenerator(const unsigned long _seed): seed((_seed ^ 0x5DEECE66DUL) & ((1UL << 48) - 1)) {}

  uint32_t java_rand31() {
    seed = (seed * 0x5DEECE66DUL + 0xB) & ((1UL << 48) - 1);
    return seed >> (48 - 31);
  }

  unsigned int uniform_choice(uint32_t bound) {
    // assert(bound > 0 && bound <= (1U << 31));
    uint32_t boundm1 = bound-1;
    if ((bound & boundm1) == 0)
      return (bound * static_cast<uint64_t>(java_rand31())) >> 31;
    uint32_t bits, val;
    do {
        bits = java_rand31();
        val = bits % bound;
    } while ((bits-val+boundm1) & (1U << 31));
    return val;
  } 
};

#elif RBG_RANDOM_GENERATOR == 2

// One of the fastest unbiased generators.
// The Lemire's method combined with boost mt11213b.
// Source: https://www.pcg-random.org/posts/bounded-rands.html
#include <boost/random/mersenne_twister.hpp>
struct RBGRandomGenerator {
  boost::random::mt11213b random_generator;
  RBGRandomGenerator(const unsigned long seed): random_generator(seed) {}

  unsigned int uniform_choice(const uint32_t upper_bound) {
    uint32_t x = random_generator();
    uint64_t m = uint64_t(x) * uint64_t(upper_bound);
    uint32_t l = uint32_t(m);
    if (l < upper_bound) {
        uint32_t t = -upper_bound;
        if (t >= upper_bound) {
            t -= upper_bound;
            if (t >= upper_bound)
                t %= upper_bound;
        }
        while (l < t) {
            x = random_generator();
            m = uint64_t(x) * uint64_t(upper_bound);
            l = uint32_t(m);
        }
    }
    return m >> 32;
  }
};

#else

// Default method
#include <random>
struct RBGRandomGenerator {
  std::mt19937 random_generator;
  RBGRandomGenerator(const unsigned long seed): random_generator(seed) {}
  unsigned int uniform_choice(const unsigned int upper_bound) {
    return std::uniform_int_distribution<unsigned int>(0,upper_bound-1)(random_generator);
  }
};

#endif
#endif
