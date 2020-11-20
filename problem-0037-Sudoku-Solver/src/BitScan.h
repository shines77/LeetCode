
#ifndef JSTD_SUPPORT_BITSCAN_H
#define JSTD_SUPPORT_BITSCAN_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

//#include <stdint.h>
#include <assert.h>

#if (defined(_MSC_VER) && (_MSC_VER >= 1500)) || defined(__MINGW32__) || defined(__CYGWIN__)
	#include <intrin.h>
#endif

#if (defined(__GNUC__) && (__GNUC__ * 1000 + __GNUC_MINOR__ >= 4005)) \
 || (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 900)) || defined(__clang__)
	#include <x86intrin.h>
#endif

#if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
#define JSTD_IS_X86_64      1
#define JSTD_WORD_BITS      64
#else
#define JSTD_IS_X86_64      0
#define JSTD_WORD_BITS      32
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1500) // >= VC 2008
    #pragma intrinsic(_BitScanReverse)
    #pragma intrinsic(_BitScanForward)
    #if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) || defined(_M_IA64)
        #pragma intrinsic(_BitScanReverse64)
        #pragma intrinsic(_BitScanForward64)
    #endif
#endif // (_MSC_VER && _MSC_VER >= 1500)

namespace jstd {

#if (defined(_MSC_VER) && (_MSC_VER >= 1500)) || defined(__MINGW32__) || defined(__CYGWIN__)
typedef __int64             int64_t;
typedef unsigned __int64    uint64_t;
#else
typedef long long           int64_t;
typedef unsigned long long  uint64_t;
#endif // (_MSC_VER && _MSC_VER >= 1500)

#if (JSTD_WORD_BITS == 64)
typedef int64_t         ssize_t;
typedef uint64_t        size_t;
#else
typedef int             ssize_t;
typedef unsigned int    size_t;
#endif

struct bitscan {

    //
    // popcount() algorithm
    //
    // See: http://www.cnblogs.com/Martinium/articles/popcount.html
    // See: https://en.wikipedia.org/wiki/Hamming_weight
    // See: https://stackoverflow.com/questions/757059/position-of-least-significant-bit-that-is-set
    //

    static inline
    unsigned int __internal_popcnt(unsigned int x) { 
        x -=  ((x >> 1U) & 0x55555555U);
        x  = (((x >> 2U) & 0x33333333U) + (x & 0x33333333U));
        x  = (((x >> 4U) + x) & 0x0F0F0F0FU);
        x +=   (x >> 8U);
        x +=   (x >> 16U);
        x  = x & 0x0000003FU;
        assert(x >= 0 && x <= 32);
        return x;
    }

    static inline
    unsigned int __internal_popcnt_slow(unsigned int x) {
        x = (x & 0x55555555UL) + ((x >>  1U) & 0x55555555UL);
        x = (x & 0x33333333UL) + ((x >>  2U) & 0x33333333UL);
        x = (x & 0x0F0F0F0FUL) + ((x >>  4U) & 0x0F0F0F0FUL);
        x = (x & 0x00FF00FFUL) + ((x >>  8U) & 0x00FF00FFUL);
        x = (x & 0x0000FFFFUL) + ((x >> 16U) & 0x0000FFFFUL);
        assert(x >= 0 && x <= 32);
        return x;
    }

    static inline
    unsigned int __internal_hakmem_popcnt(unsigned int x) {
        unsigned int tmp;
        tmp = x - ((x >> 1) & 033333333333) - ((x >> 2) & 011111111111);
        return (((tmp + (tmp >> 3)) & 030707070707) % 63U);
    }

    static inline
    unsigned int __internal_popcnt64(uint64_t x) {
    #if 1
        x -=  ((x >> 1U) & 0x55555555U);
        x  = (((x >> 2U) & 0x33333333U) + (x & 0x33333333U));
        x  = (((x >> 4U) + x) & 0x0F0F0F0FU);
        x +=   (x >> 8U);
        x +=   (x >> 16U);
        x +=   (x >> 32U);
        x  = x & 0x0000007FU;
        assert(x >= 0 && x <= 64);
        return (unsigned int)x;
    #elif 0
        x = (x & 0x5555555555555555ULL) + ((x >>  1U) & 0x5555555555555555ULL);
        x = (x & 0x3333333333333333ULL) + ((x >>  2U) & 0x3333333333333333ULL);
        x = (x & 0x0F0F0F0F0F0F0F0FULL) + ((x >>  4U) & 0x0F0F0F0F0F0F0F0FULL);
        x = (x & 0x00FF00FF00FF00FFULL) + ((x >>  8U) & 0x00FF00FF00FF00FFULL);
        x = (x & 0x0000FFFF0000FFFFULL) + ((x >> 16U) & 0x0000FFFF0000FFFFULL);
        x = (x & 0x00000000FFFFFFFFULL) + ((x >> 32U) & 0x00000000FFFFFFFFULL);
        assert(x >= 0 && x <= 64);
        return (unsigned int)x;
    #else
        unsigned int high, low;
        unsigned int n1, n2;
        high = (unsigned int) (x & 0x00000000FFFFFFFFULL);
        low  = (unsigned int)((x & 0xFFFFFFFF00000000ULL) >> 32U);
        n1 = __internal_popcnt(high);
        n2 = __internal_popcnt(low);
        return (n1 + n2);
    #endif
    }

    static inline
    int __internal_clz(unsigned int x) {
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        return (int)(32U - __internal_popcnt(x));
    }

    static inline
    int __internal_clzll(uint64_t x) {
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        x |= (x >> 32);
        return (int)(64U - __internal_popcnt64(x));
    }

    static inline
    int __internal_ctz(unsigned int x) {
        return (int)__internal_popcnt((x & -(int)x) - 1);
    }

    static inline
    int __internal_ctzll(uint64_t x) {
        return (int)__internal_popcnt64((x & -(int64_t)x) - 1);
    }

#if (defined(_MSC_VER) && (_MSC_VER >= 1500)) || defined(__MINGW32__) || defined(__CYGWIN__)

    static inline
    int bsf32(unsigned int x) {
        assert(x != 0);
        unsigned long index;
        ::_BitScanForward(&index, (unsigned long)x);
        return (int)index;
    }

#if (JSTD_WORD_BITS == 64)
    static inline
    int bsf64(unsigned __int64 x) {
        assert(x != 0);
        unsigned long index;
        ::_BitScanForward64(&index, x);
        return (int)index;
    }
#endif

    static inline
    int bsr32(unsigned int x) {
        assert(x != 0);
        unsigned long index;
        ::_BitScanReverse(&index, (unsigned long)x);
        return (int)index;
    }

#if (JSTD_WORD_BITS == 64)
    static inline
    int bsr64(unsigned __int64 x) {
        assert(x != 0);
        unsigned long index;
        ::_BitScanReverse64(&index, x);
        return (int)index;
    }
#endif

#elif (defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4)))) \
   || (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 900)) \
   || defined(__clang__)

    static inline
    int bsf32(unsigned int x) {
        assert(x != 0);
        return __builtin_ctz(x);
    }

#if (JSTD_WORD_BITS == 64)
    static inline
    int bsf64(unsigned long long x) {
        assert(x != 0);
        return __bsfq(x);
    }
#endif

    static inline
    int bsr32(unsigned int x) {
        assert(x != 0);
        return __builtin_clz(x);
    }

#if (JSTD_WORD_BITS == 64)
    static inline
    int bsr64(unsigned long long x) {
        assert(x != 0);
        return __bsrq(x);
    }
#endif

#else

    static inline
    int bsf32(unsigned int x) {
        assert(x != 0);
        return bitscan::__internal_ctz(x);
    }

    static inline
    int bsf64(unsigned long long x) {
        assert(x != 0);
        return bitscan::__internal_ctzll(x);
    }

    static inline
    int bsr32(unsigned int x) {
        assert(x != 0);
        return bitscan::__internal_clz(x);
    }

    static inline
    int bsr64(unsigned long long x) {
        assert(x != 0);
        return bitscan::__internal_clzll(x);
    }

#endif // (_MSC_VER && _MSC_VER >= 1500)

    static inline int bsf(size_t x) {
#if (JSTD_WORD_BITS == 64)
        return bitscan::bsf64(x);
#else
        return bitscan::bsf32(x);
#endif
    }

    static inline int bsr(size_t x) {
#if (JSTD_WORD_BITS == 64)
        return bitscan::bsr64(x);
#else
        return bitscan::bsr32(x);
#endif
    }

};

} // namespace jstd

#undef JSTD_IS_X86_64
#undef JSTD_WORD_BITS

#endif // JSTD_SUPPORT_BITSCAN_H
