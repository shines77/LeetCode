
#ifndef LEETCODE_SUDOKU_SOLVER_H
#define LEETCODE_SUDOKU_SOLVER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <bitset>
#include <cstring>      // For std::memset()
#include <initializer_list>
#include <type_traits>
#include <cassert>

#include "BitUtils.h"

#define MATRIX_USE_STD_BITSET       0
#define MATRIX_USE_SMALL_BITSET     1
#define MATRIX_USE_BITMAP           2
#define MATRIX_USE_BITSET           3

#if defined(_MSC_VER)
  #define MATRIX_BITSET_MODE        MATRIX_USE_STD_BITSET
#else
  #define MATRIX_BITSET_MODE        MATRIX_USE_STD_BITSET
#endif

namespace LeetCode {
namespace Problem_37 {

struct dont_init_t {};

template <size_t Bits, bool NeedTrim = true>
class SmallBitSet {
public:
    typedef typename std::conditional<
                (Bits <= sizeof(uint32_t) * 8), uint32_t, size_t
            >::type  unit_type;
    typedef SmallBitSet<Bits, NeedTrim> this_type;

    static const size_t kUnitBytes = sizeof(unit_type);
    static const size_t kUnitBits  = 8 * kUnitBytes;
    static const size_t kUnits = (Bits + kUnitBits - 1) / kUnitBits;
    static const size_t kBytes = kUnits * kUnitBytes;
    static const size_t kBits  = kUnits * kUnitBits;
    static const size_t kRestBits = (Bits % kUnitBits);
    static const unit_type kFullMask = unit_type(-1);
    static const unit_type kTrimMask = (kRestBits != 0) ? (unit_type(size_t(1) << kRestBits) - 1) : kFullMask;

private:
    unit_type array_[kUnits];

public:
    SmallBitSet() noexcept {
        static_assert((Bits != 0), "SmallBitSet<Bits>: Bits can not be 0 size.");
        this->reset();
    }

    SmallBitSet(dont_init_t & dont_init) noexcept {
        static_assert((Bits != 0), "SmallBitSet<Bits>: Bits can not be 0 size.");
    }

    SmallBitSet(const SmallBitSet<Bits, NeedTrim> & src) noexcept {
        for (size_t i = 0; i < kUnits; i++) {
            this->array_[i] = src.array(i);
        }
    }

    template <size_t UBits, bool UNeedTrim>
    SmallBitSet(const SmallBitSet<UBits, UNeedTrim> & src) noexcept {
        typedef SmallBitSet<UBits, UNeedTrim> SourceBitMap;
        size_t copyUnits = std::min(kUnits, SourceBitMap::kUnits);
        for (size_t i = 0; i < copyUnits; i++) {
            this->array_[i] = src.array(i);
        }
    }

    SmallBitSet(unit_type value) noexcept {
        this->array_[0] = value;
    }

    SmallBitSet(std::initializer_list<unit_type> init_list) noexcept {
        if (init_list.size() <= kUnits) {
            size_t i = 0;
            for (auto iter : init_list) {
                this->array_[i++] = *iter;
            }
        }
        else {
            size_t i = 0;
            for (auto iter : init_list) {
                this->array_[i++] = *iter;
                if (i >= kUnits) {
                    break;
                }
            }
        }
    }

    ~SmallBitSet() = default;

    size_t size() const        { return Bits; }

          char * data()        { return (char *)      this->array_; }
    const char * data() const  { return (const char *)this->array_; }

    size_t total_bytes() const { return kBytes; }
    size_t unit_size() const { return kUnits; }
    size_t per_unit_bytes() const { return sizeof(unit_type); }

    bool need_trim() const { return NeedTrim; }

    size_t array(size_t index) const {
        assert(index < kUnits);
        return this->array_[index];
    }

private:
    template <size_t Pos>
    inline bool tail_is_any() const noexcept {
        if (need_trim() && (kRestBits != 0)) {
            size_t unit = this->array_[Pos] & kTrimMask;
            return (unit != 0);
        }
        else {
            return (this->array_[Pos] != 0);
        }
    }

    template <size_t Pos>
    inline bool tail_is_none() const noexcept {
        if (need_trim() && (kRestBits != 0)) {
            size_t unit = this->array_[Pos] & kTrimMask;
            return (unit == 0);
        }
        else {
            return (this->array_[Pos] == 0);
        }
    }

    template <size_t Pos>
    inline bool tail_is_all() const noexcept {
        if (need_trim() && (kRestBits != 0)) {
            size_t unit = this->array_[Pos] & kTrimMask;
            return (unit == kTrimMask);
        }
        else {
            return (this->array_[Pos] == kTrimMask);
        }
    }

public:
    this_type & init(std::initializer_list<unit_type> init_list) noexcept {
        if (init_list.size() <= kUnits) {
            size_t i = 0;
            for (auto iter : init_list) {
                this->array_[i++] = *iter;
            }
        }
        else {
            size_t i = 0;
            for (auto iter : init_list) {
                this->array_[i++] = *iter;
                if (i >= kUnits) {
                    break;
                }
            }
        }
        return (*this);
    }

    class reference {
    private:
        this_type * bitmap_;    // pointer to the bitmap
        size_t pos_;            // position of element in bitset

        // proxy for an element
        friend class SmallBitSet<Bits, NeedTrim>;

    public:
        ~reference() noexcept {
            // destroy the object
        }

        reference & operator = (bool value) noexcept {
            // assign Boolean to element
            this->bitmap_->set(pos_, value);
            return (*this);
        }

        reference & operator = (const reference & right) noexcept {
            // assign reference to element
            this->bitmap_->set(pos_, bool(right));
            return (*this);
        }

        reference & flip() noexcept {
            // complement stored element
            this->bitmap_->flip(pos_);
            return (*this);
        }

        bool operator ~ () const noexcept {
            // return complemented element
            return (!this->bitmap_->test(pos_));
        }

        bool operator ! () const noexcept {
            // return complemented element
            return (!this->bitmap_->test(pos_));
        }

        operator bool () const noexcept {
            // return element
            return (this->bitmap_->test(pos_));
        }

    private:
        reference() noexcept
            : bitmap_(nullptr), pos_(0) {
            // default construct
        }

        reference(this_type & bitmap, size_t pos) noexcept
            : bitmap_(&bitmap), pos_(pos) {
            // construct from bitmap reference and position
        }
    };

    this_type & operator = (const this_type & right) noexcept {
        for (size_t i = 0; i < kUnits; i++) {
            this->array_[i] = right.array(i);
        }
        return (*this);
    }

    constexpr bool operator [] (size_t pos) const {
        assert(pos < Bits);
        return this->test(pos);
    }

    reference operator [] (size_t pos) {
        assert(pos < Bits);
        return reference(*this, pos);
    }

    this_type & operator & (unit_type value) noexcept {
        this->array_[0] &= value;
        return (*this);
    }

    this_type & operator | (unit_type value) noexcept {
        this->array_[0] |= value;
        return (*this);
    }

    this_type & operator ^ (unit_type value) noexcept {
        this->array_[0] ^= value;
        return (*this);
    }

    this_type & operator &= (unit_type value) noexcept {
        this->array_[0] &= value;
        return (*this);
    }

    this_type & operator |= (unit_type value) noexcept {
        this->array_[0] |= value;
        return (*this);
    }

    this_type & operator ^= (unit_type value) noexcept {
        this->array_[0] ^= value;
        return (*this);
    }

    this_type & operator &= (const this_type & right) noexcept {
        for (size_t i = 0; i < kUnits; i++) {
            this->array_[i] &= right.array(i);
        }
        return (*this);
    }

    this_type & operator |= (const this_type & right) noexcept {
        for (size_t i = 0; i < kUnits; i++) {
            this->array_[i] |= right.array(i);
        }
        return (*this);
    }

    this_type & operator ^= (const this_type & right) noexcept {
        for (size_t i = 0; i < kUnits; i++) {
            this->array_[i] ^= right.array(i);
        }
        return (*this);
    }

    this_type operator ~ () const noexcept {
        // Flip all bits
        return (this_type(*this).flip());
    }

    this_type operator ! () const noexcept {
        // Flip all bits
        return (this_type(*this).flip());
    }

    bool operator == (const this_type & right) noexcept {
        for (size_t i = 0; i < kUnits; i++) {
            if (this->array_[i] != right.array(i)) {
                return false;
            }
        }
        return true;
    }

    bool operator != (const this_type & right) noexcept {
        for (size_t i = 0; i < kUnits; i++) {
            if (this->array_[i] == right.array(i)) {
                return false;
            }
        }
        return true;
    }

    this_type & fill(size_t value) noexcept {
        for (size_t i = 0; i < kUnits; i++) {
            this->array_[i] = (unit_type)value;
        }
        if (need_trim()) {
            this->trim();
        }
        return (*this);
    }

    this_type & set() noexcept {
        if (kUnits <= 8) {
            for (size_t i = 0; i < kUnits; i++) {
                this->array_[i] = kFullMask;
            }
        }
        else {
            std::memset(this->array_, (kFullMask & 0xFF), kUnits * sizeof(unit_type));
        }
        if (need_trim()) {
            this->trim();
        }
        return (*this);
    }

    this_type & set(size_t pos) {
        assert(pos < Bits);
        if (Bits <= kUnitBits) {
            this->array_[0] |= unit_type(size_t(1) << pos);
        }
        else {
            size_t index = pos / kUnitBits;
            size_t shift = pos % kUnitBits;
            this->array_[index] |= unit_type(size_t(1) << shift);
        }
        return (*this);
    }

    this_type & set(size_t pos, bool value) {
        if (value)
            this->set(pos);
        else
            this->reset(pos);
        return (*this);
    }

    this_type & reset() noexcept {
        if (kUnits <= 8) {
            for (size_t i = 0; i < kUnits; i++) {
                this->array_[i] = 0;
            }
        }
        else {
            std::memset(this->array_, 0, kUnits * sizeof(unit_type));
        }
        return (*this);
    }

    this_type & reset(size_t pos) {
        assert(pos < Bits);
        if (Bits <= kUnitBits) {
            this->array_[0] &= unit_type(~(size_t(1) << pos));
        }
        else {
            size_t index = pos / kUnitBits;
            size_t shift = pos % kUnitBits;
            this->array_[index] &= unit_type(~(size_t(1) << shift));
        }
        return (*this);
    }

    this_type & flip() noexcept {
        for (size_t i = 0; i < kUnits; i++) {
            this->array_[i] ^= unit_type(-1);
        }
        if (need_trim()) {
            this->trim();
        }
        return (*this);
    }

    this_type & flip(size_t pos) {
        assert(pos < Bits);
        if (Bits <= kUnitBits) {
            this->array_[0] ^= unit_type(~(size_t(1) << pos));
        }
        else {
            size_t index = pos / kUnitBits;
            size_t shift = pos % kUnitBits;
            this->array_[index] ^= unit_type(~(size_t(1) << shift));
        }
        return (*this);
    }

    this_type & trim() noexcept {
        if (kRestBits != 0) {
            this->array_[kUnits - 1] &= kTrimMask;
        }
        return (*this);
    }

    bool test(size_t pos) const {
        assert(pos < Bits);
        if (Bits <= kUnitBits) {
            return ((this->array_[0] & unit_type(size_t(1) << pos)) != 0);
        }
        else {
            size_t index = pos / kUnitBits;
            size_t shift = pos % kUnitBits;
            return ((this->array_[index] & unit_type(size_t(1) << shift)) != 0);
        }
    }

    size_t value(size_t pos) const {
        assert(pos < Bits);
        if (Bits <= kUnitBits) {
            return (this->array_[0] & unit_type(size_t(1) << pos));
        }
        else {
            size_t index = pos / kUnitBits;
            size_t shift = pos % kUnitBits;
            return (this->array_[index] & unit_type(size_t(1) << shift));
        }
    }

    bool any() const noexcept {
        if (Bits <= kUnitBits) {
            return tail_is_any<0>();
        }
        else {
            for (size_t i = 0; i < kUnits - 1; i++) {
                size_t unit = this->array_[i];
                if (unit != 0) {
                    return true;
                }
            }
            return tail_is_any<kUnits - 1>();
        }
    }

    bool none() const noexcept {
#if 1
        return !(this->any());
#else
        if (Bits <= kUnitBits) {
            return tail_is_none<0>();
        }
        else {
            for (size_t i = 0; i < kUnits - 1; i++) {
                size_t unit = this->array_[i];
                if (unit != 0) {
                    return false;
                }
            }
            return tail_is_none<kUnits - 1>();
        }
#endif
    }

    bool all() const noexcept {
        if (Bits <= kUnitBits) {
            return tail_is_all<0>();
        }
        else {
            for (size_t i = 0; i < kUnits - 1; i++) {
                size_t unit = this->array_[i];
                if (unit != kFullMask) {
                    return false;
                }
            }
            return tail_is_all<kUnits - 1>();
        }
    }

    size_t bsf() const noexcept {
        for (size_t i = 0; i < kUnits; i++) {
            size_t unit = this->array_[i];
            if (unit != 0) {
                unsigned int index = jstd::BitUtils::bsf(unit);
                return (i * kUnitBits + index);
            }
        }
        return 0;
    }

    size_t bsr() const noexcept {
        for (ptrdiff_t i = kUnits - 1; i >= 0; i--) {
            size_t unit = this->array_[i];
            if (unit != 0) {
                unsigned int index = jstd::BitUtils::bsr(unit);
                return size_t(i * kUnitBits + index);
            }
        }
        return Bits;
    }

    size_t count() const noexcept {
        size_t total_popcnt = 0;
        for (size_t i = 0; i < kUnits; i++) {
            size_t unit = this->array_[i];
            unsigned int popcnt = jstd::BitUtils::popcnt(unit);
            total_popcnt += popcnt;
        }
        return total_popcnt;
    }

    unsigned long to_ulong() const {
        if (Bits <= sizeof(uint32_t) * 8) {
            return this->array_[0];
        }
        else {
            return static_cast<unsigned long>(this->array_[0]);
        }
    }

    uint64_t to_ullong() const {
        if (Bits <= sizeof(uint32_t) * 8) {
            return static_cast<uint64_t>(this->array_[0]);
        }
        else {
            return this->array_[0];
        }
    }
};

template <size_t Bits, bool NeedTrim>
inline
SmallBitSet<Bits, NeedTrim> operator & (const SmallBitSet<Bits, NeedTrim> & left,
                                        const SmallBitSet<Bits, NeedTrim> & right) noexcept {
    // left And right
    SmallBitSet<Bits, NeedTrim> answer = left;
    return (answer &= right);
}

template <size_t Bits, bool NeedTrim>
inline
SmallBitSet<Bits, NeedTrim> operator | (const SmallBitSet<Bits, NeedTrim> & left,
                                        const SmallBitSet<Bits, NeedTrim> & right) noexcept {
    // left Or right
    SmallBitSet<Bits, NeedTrim> answer = left;
    return (answer |= right);
}

template <size_t Bits, bool NeedTrim>
inline
SmallBitSet<Bits, NeedTrim> operator ^ (const SmallBitSet<Bits, NeedTrim> & left,
                                        const SmallBitSet<Bits, NeedTrim> & right) noexcept {
    // left Xor right
    SmallBitSet<Bits, NeedTrim> answer = left;
    return (answer ^= right);
}

template <size_t Bits>
class BitSet {
public:
    typedef typename std::conditional<(Bits <= sizeof(uint32_t) * 8), uint32_t, size_t>::type  unit_type;

    static const size_t kUnitBits = 8 * sizeof(unit_type);
    static const size_t kUnits = (Bits + kUnitBits - 1) / kUnitBits;
    static const size_t kBytes = kUnits * kUnitBits;

private:
    unit_type * bits_;

public:
    BitSet() : bits_(nullptr) {
        this->bits_ = new unit_type[kUnits];
    }
    ~BitSet() {
        if (this->bits_ != nullptr) {
            delete[] this->bits_;
            this->bits_ = nullptr;
        }
    }

    size_t size() const        { return Bits; }

          char * data()        { return (char *)      bits_; }
    const char * data() const  { return (const char *)bits_; }

    size_t bytes() const { return kBytes; }
    size_t units() const { return kUnits; }
    size_t unit_length() const { return sizeof(unit_type); }

    void clear() {
        if (this->bits_) {
            ::memset(this->bits_, 0, kUnits * sizeof(unit_type));
        }
    }

    bool test(size_t pos) const {
        assert(pos < Bits);
        size_t index = pos / kUnitBits;
        size_t shift = pos % kUnitBits;
        return ((this->bits_[index] & unit_type(size_t(1) << shift)) != 0);
    }

    size_t value(size_t pos) const {
        assert(pos < Bits);
        size_t index = pos / kUnitBits;
        size_t shift = pos % kUnitBits;
        return (this->bits_[index] & unit_type(size_t(1) << shift));
    }

    void set() {
        for (size_t i = 0; i < kUnits; i++) {
            this->bits_[i] = unit_type(-1);
        }
    }

    void set(size_t pos) {
        assert(pos < Bits);
        size_t index = pos / kUnitBits;
        size_t shift = pos % kUnitBits;
        this->bits_[index] |= unit_type(size_t(1) << shift);
    }

    void set(size_t pos, bool value) {
        if (value)
            this->set(pos);
        else
            this->reset(pos);
    }

    void reset() {
        for (size_t i = 0; i < kUnits; i++) {
            this->bits_[i] = 0;
        }
    }

    void reset(size_t pos) {
        assert(pos < Bits);
        size_t index = pos / kUnitBits;
        size_t shift = pos % kUnitBits;
        this->bits_[index] &= unit_type(~(size_t(1) << shift));
    }

    void flip() {
        for (size_t i = 0; i < kUnits; i++) {
            this->bits_[i] ^= unit_type(-1);
        }
    }

    void flip(size_t pos) {
        assert(pos < Bits);
        size_t index = pos / kUnitBits;
        size_t shift = pos % kUnitBits;
        this->bits_[index] ^= unit_type(~(size_t(1) << shift));
    }

    void trim() {
		this->bits_[kUnits - 1] &= unit_type(size_t(1) << (Bits % kUnitBits)) - 1;
    }
};

template <size_t Bits>
class BitMap {
public:
    static const size_t kAlignmentBytes = sizeof(size_t);
    static const size_t kBytes = ((Bits + kAlignmentBytes - 1) / kAlignmentBytes) * kAlignmentBytes;

private:
    uint8_t * bytes_;

public:
    BitMap() : bytes_(nullptr) {
        this->bytes_ = new uint8_t[kBytes];
    }
    ~BitMap() {
        if (this->bytes_ != nullptr) {
            delete[] this->bytes_;
            this->bytes_ = nullptr;
        }
    }

    size_t size() const        { return Bits; }

          char * data()        { return (      char *)bytes_; }
    const char * data() const  { return (const char *)bytes_; }

    size_t bytes() const { return kBytes; }

    void clear() {
        if (this->bytes_) {
            ::memset(this->bytes_, 0, kBytes * sizeof(uint8_t));
        }
    }

    bool test(size_t pos) const {
        assert(pos < Bits);
        return (this->bytes_[pos] != 0);
    }

    uint8_t value(size_t pos) const {
        assert(pos < Bits);
        return this->bytes_[pos];
    }

    void set() {
        for (size_t i = 0; i < kBytes; i++) {
            this->bytes_[i] = uint8_t(-1);
        }
    }

    void set(size_t pos) {
        assert(pos < Bits);
        this->bytes_[pos] = uint8_t(-1);
    }

    void set(size_t pos, bool value) {
        if (value)
            this->set(pos);
        else
            this->reset(pos);
    }

    void reset() {
        for (size_t i = 0; i < kBytes; i++) {
            this->bytes_[i] = 0;
        }
    }

    void reset(size_t pos) {
        assert(pos < Bits);
        this->bytes_[pos] = 0;
    }

    void flip() {
        for (size_t i = 0; i < kBytes; i++) {
            this->bytes_[i] ^= uint8_t(-1);
        }
    }

    void flip(size_t pos) {
        assert(pos < Bits);
        this->bytes_[pos] ^= uint8_t(-1);
    }

    void trim() {
		/* Not support */
    }

    uint8_t & operator [] (size_t pos) {
        assert(pos < Bits);
        return this->bytes_[pos];
    }

    const uint8_t operator [] (size_t pos) const {
        assert(pos < Bits);
        return this->bytes_[pos];
    }
};

template <size_t Rows, size_t Cols>
class SmallBitMatrix {
private:
#if (MATRIX_BITSET_MODE == MATRIX_USE_SMALL_BITSET)
    typedef SmallBitSet<Cols>   bitmap_type;
#elif (MATRIX_BITSET_MODE == MATRIX_USE_BITSET)
    typedef BitSet<Cols>        bitmap_type;
#elif (MATRIX_BITSET_MODE == MATRIX_USE_BITMAP)
    typedef BitMap<Cols>        bitmap_type;
#else
    typedef std::bitset<Cols>   bitmap_type;
#endif
    size_t rows_;
    bitmap_type array_[Rows];

public:
    SmallBitMatrix() : rows_(Rows) {}
    ~SmallBitMatrix() {}

    size_t rows() const { return this->rows_; }
    size_t cols() const { return Cols; }

    size_t size() const { return Rows; }
    size_t total_size() const { return (Rows * Cols); }

    void setRows(size_t rows) {
        this->rows_ = rows;
    }

    void clear() {
#if (MATRIX_BITSET_MODE != MATRIX_USE_STD_BITSET)
        for (size_t row = 0; row < Rows; row++) {
            this->array_[row].clear();
        }
#endif
    }

    bool test(size_t row, size_t col) {
        assert(row < Rows);
        return this->array_[row].test(col);
    }

    size_t value(size_t row, size_t col) {
        assert(row < Rows);
#if (MATRIX_BITSET_MODE != MATRIX_USE_STD_BITSET)
        return this->array_[row].value(col);
#else
        return (size_t)(this->array_[row].test(col));
#endif
    }

    void set() {
        for (size_t row = 0; row < Rows; row++) {
            this->array_[row].set();
#if (MATRIX_BITSET_MODE != MATRIX_USE_STD_BITSET)
            this->array_[row].trim();
#endif
        }
    }

    void reset() {
        for (size_t row = 0; row < Rows; row++) {
            this->array_[row].reset();
        }
    }

    void flip() {
        for (size_t row = 0; row < Rows; row++) {
            this->array_[row].flip();
        }
    }

    void trim() {
#if (MATRIX_BITSET_MODE != MATRIX_USE_STD_BITSET)
        for (size_t row = 0; row < Rows; row++) {
            this->array_[row].trim();
        }
#endif
    }

    bitmap_type & operator [] (size_t pos) {
        assert(pos < Rows);
        return this->array_[pos];
    }

    const bitmap_type & operator [] (size_t pos) const {
        assert(pos < Rows);
        return this->array_[pos];
    }
};

template <size_t Rows, size_t Cols>
class SmallBitMatrix2 {
private:
    typedef std::bitset<Cols>   bitset_type;

    bitset_type array_[Rows];

public:
    SmallBitMatrix2() = default;
    ~SmallBitMatrix2() = default;

    size_t rows() const { return Rows; }
    size_t cols() const { return Cols; }

    size_t size() const { return Rows; }
    size_t total_size() const { return (Rows * Cols); }

    bool test(size_t row, size_t col) {
        assert(row < Rows);
        return this->array_[row].test(col);
    }

    void set() {
        for (size_t row = 0; row < Rows; row++) {
            this->array_[row].set();
        }
    }

    void reset() {
        for (size_t row = 0; row < Rows; row++) {
            this->array_[row].reset();
        }
    }

    void flip() {
        for (size_t row = 0; row < Rows; row++) {
            this->array_[row].flip();
        }
    }

    bitset_type & operator [] (size_t pos) {
        assert(pos < Rows);
        return this->array_[pos];
    }

    const bitset_type & operator [] (size_t pos) const {
        assert(pos < Rows);
        return this->array_[pos];
    }
};

template <size_t Depths, size_t Rows, size_t Cols>
class SmallBitMatrix3 {
private:
    typedef SmallBitMatrix2<Rows, Cols>  matrix_type;

    matrix_type matrix_[Depths];

public:
    SmallBitMatrix3() = default;
    ~SmallBitMatrix3() = default;

    size_t depths() const { return Depths; }
    size_t rows() const { return Rows; }
    size_t cols() const { return Cols; }

    size_t size() const { return Depths; }
    size_t matrix2d_size() const { return (Rows * Cols); }
    size_t total_size() const { return (Depths * Rows * Cols); }

    bool test(size_t depth, size_t row, size_t col) {
        assert(depth < Depths);
        return this->matrix_[depth][row].test(col);
    }

    void set() {
        for (size_t depth = 0; depth < Depths; depth++) {
            this->matrix_[depth].set();
        }
    }

    void reset() {
        for (size_t depth = 0; depth < Depths; depth++) {
            this->matrix_[depth].reset();
        }
    }

    void flip() {
        for (size_t depth = 0; depth < Depths; depth++) {
            this->matrix_[depth].flip();
        }
    }

    matrix_type & operator [] (size_t pos) {
        assert(pos < Depths);
        return this->matrix_[pos];
    }

    const matrix_type & operator [] (size_t pos) const {
        assert(pos < Depths);
        return this->matrix_[pos];
    }
};

template <size_t Rows, size_t Cols>
class BitMatrix2 {
private:
    typedef std::bitset<Cols>   bitset_type;

    std::vector<bitset_type> array_;

public:
    BitMatrix2() {
        this->array_.resize(Rows);
    }

    BitMatrix2(const BitMatrix2 & src) {
        this->array_.reserve(Rows);
        for (size_t row = 0; row < Rows; row++) {
            this->array_.push_back(src[row]);
        }
    }

    BitMatrix2(const SmallBitMatrix2<Rows, Cols> & src) {
        this->array_.reserve(Rows);
        for (size_t row = 0; row < Rows; row++) {
            this->array_.push_back(src[row]);
        }
    }

    ~BitMatrix2() = default;

    BitMatrix2 & operator = (const BitMatrix2 & rhs) {
        if (&rhs != this) {
            for (size_t row = 0; row < Rows; row++) {
                this->array_[row] = rhs[row];
            }
        }
    }

    BitMatrix2 & operator = (const SmallBitMatrix2<Rows, Cols> & rhs) {
        for (size_t row = 0; row < Rows; row++) {
            this->array_[row] = rhs[row];
        }
    }

    size_t rows() const { return Rows; }
    size_t cols() const { return Cols; }

    size_t size() const { return Rows; }
    size_t total_size() const { return (Rows * Cols); }

    bool test(size_t row, size_t col) {
        assert(row < Rows);
        return this->array_[row].test(col);
    }

    void set() {
        for (size_t row = 0; row < Rows; row++) {
            this->array_[row].set();
        }
    }

    void reset() {
        for (size_t row = 0; row < Rows; row++) {
            this->array_[row].reset();
        }
    }

    void flip() {
        for (size_t row = 0; row < Rows; row++) {
            this->array_[row].flip();
        }
    }

    bitset_type & operator [] (size_t pos) {
        assert(pos < Rows);
        return this->array_[pos];
    }

    const bitset_type & operator [] (size_t pos) const {
        assert(pos < Rows);
        return this->array_[pos];
    }
};

template <size_t Depths, size_t Rows, size_t Cols>
class BitMatrix3 {
private:
    typedef BitMatrix2<Rows, Cols>  matrix_type;

    std::vector<matrix_type> matrix_;

public:
    BitMatrix3() {
        this->matrix_.resize(Depths);
    }

    BitMatrix3(const BitMatrix3 & src) {
        this->matrix_.reserve(Depths);
        for (size_t depth = 0; depth < Depths; depth++) {
            this->matrix_.push_back(src[depth]);
        }
    }

    BitMatrix3(const SmallBitMatrix3<Depths, Rows, Cols> & src) {
        this->matrix_.reserve(Depths);
        for (size_t depth = 0; depth < Depths; depth++) {
            this->matrix_.push_back(src[depth]);
        }
    }

    ~BitMatrix3() = default;

    BitMatrix3 & operator = (const BitMatrix3 & rhs) {
        if (&rhs != this) {
            for (size_t depth = 0; depth < Depths; depth++) {
                this->matrix_[depth] = rhs[depth];
            }
        }
    }

    BitMatrix3 & operator = (const SmallBitMatrix3<Depths, Rows, Cols> & rhs) {
        for (size_t depth = 0; depth < Depths; depth++) {
            this->matrix_[depth] = rhs[depth];
        }
    }

    size_t depths() const { return Depths; }
    size_t rows() const { return Rows; }
    size_t cols() const { return Cols; }

    size_t size() const { return Depths; }
    size_t matrix2d_size() const { return (Rows * Cols); }
    size_t total_size() const { return (Depths * Rows * Cols); }

    bool test(size_t depth, size_t row, size_t col) {
        assert(depth < Depths);
        return this->matrix_[depth][row].test(col);
    }

    void set() {
        for (size_t depth = 0; depth < Depths; depth++) {
            this->matrix_[depth].set();
        }
    }

    void reset() {
        for (size_t depth = 0; depth < Depths; depth++) {
            this->matrix_[depth].reset();
        }
    }

    void flip() {
        for (size_t depth = 0; depth < Depths; depth++) {
            this->matrix_[depth].flip();
        }
    }

    matrix_type & operator [] (size_t pos) {
        assert(pos < Depths);
        return this->matrix_[pos];
    }

    const matrix_type & operator [] (size_t pos) const {
        assert(pos < Depths);
        return this->matrix_[pos];
    }
};

template <size_t Rows, size_t Cols>
static void matrix2_copy(SmallBitMatrix2<Rows, Cols> & dest,
                         const BitMatrix2<Rows, Cols> & src)
{
    for (size_t row = 0; row < Rows; row++) {
        dest[row] = src[row];
    }
}

template <size_t Rows, size_t Cols>
static void matrix2_copy(BitMatrix2<Rows, Cols> & dest,
                         const SmallBitMatrix2<Rows, Cols> & src)
{
    for (size_t row = 0; row < Rows; row++) {
        dest[row] = src[row];
    }
}

template <size_t Depths, size_t Rows, size_t Cols>
static void matrix3_copy(SmallBitMatrix3<Depths, Rows, Cols> & dest,
                         const BitMatrix3<Depths, Rows, Cols> & src)
{
    for (size_t depth = 0; depth < Depths; depth++) {
        for (size_t row = 0; row < Rows; row++) {
            dest[depth][row] = src[depth][row];
        }
    }
}

template <size_t Depths, size_t Rows, size_t Cols>
static void matrix3_copy(BitMatrix3<Depths, Rows, Cols> & dest,
                         const SmallBitMatrix3<Depths, Rows, Cols> & src)
{
    for (size_t depth = 0; depth < Depths; depth++) {
        for (size_t row = 0; row < Rows; row++) {
            dest[depth][row] = src[depth][row];
        }
    }
}

template <size_t nPalaceRows = 3, size_t nPalaceCols = 3,
          size_t nPalaceCountX = 3, size_t nPalaceCountY = 3,
          size_t nMinNumber = 1, size_t nMaxNumber = 9>
struct BasicSudokuHelper {
    static const size_t PalaceRows = nPalaceRows;       // 3
    static const size_t PalaceCols = nPalaceCols;       // 3
    static const size_t PalaceCountX = nPalaceCountX;   // 3
    static const size_t PalaceCountY = nPalaceCountY;   // 3
    static const size_t MinNumber = nMinNumber;         // 1
    static const size_t MaxNumber = nMaxNumber;         // 9

    static const size_t Rows = PalaceRows * PalaceCountY;
    static const size_t Cols = PalaceCols * PalaceCountX;
    static const size_t Palaces = PalaceCountX * PalaceCountY;
    static const size_t Numbers = (MaxNumber - MinNumber) + 1;

    static const size_t PalaceSize = PalaceRows * PalaceCols;
    static const size_t BoardSize = Rows * Cols;
    static const size_t TotalSize = PalaceSize * Palaces * Numbers;

    static const size_t TotalSize2 = Rows * Cols * Numbers;

    static const size_t TotalConditions0 = 0;
    static const size_t TotalConditions1 = Rows * Cols;
    static const size_t TotalConditions2 = Rows * Numbers;
    static const size_t TotalConditions3 = Cols * Numbers;
    static const size_t TotalConditions4 = Palaces * Numbers;

    static const size_t TotalConditions01 = TotalConditions0  + TotalConditions1;
    static const size_t TotalConditions02 = TotalConditions01 + TotalConditions2;
    static const size_t TotalConditions03 = TotalConditions02 + TotalConditions3;
    static const size_t TotalConditions04 = TotalConditions03 + TotalConditions4;

    static const size_t TotalConditions =
        TotalConditions1 + TotalConditions2 + TotalConditions3 + TotalConditions4;

    static const size_t kAllRowsBit = (size_t(1) << Rows) - 1;
    static const size_t kAllColsBit = (size_t(1) << Cols) - 1;
    static const size_t kAllNumbersBit = (size_t(1) << Numbers) - 1;

    static void clear_board(std::vector<std::vector<char>> & board) {
        for (size_t row = 0; row < board.size(); row++) {
            std::vector<char> & line = board[row];
            for (size_t col = 0; col < line.size(); col++) {
                line[col] = '.';
            }
        }
    }

    static void display_board(const std::vector<std::vector<char>> & board,
                              bool is_input = false,
                              int idx = -1) {
        int filled = 0;
        for (size_t row = 0; row < board.size(); row++) {
            const std::vector<char> & line = board[row];
            for (size_t col = 0; col < line.size(); col++) {
                if (line[col] != '.')
                    filled++;
            }
        }

        if (is_input) {
            printf("The input is: (filled = %d)\n", filled);
        }
        else {
            if (idx == -1)
                printf("The answer is:\n");
            else
                printf("The answer # %d is:\n", idx + 1);
        }
        printf("\n");
        // printf("  ------- ------- -------\n");
        printf(" ");
        for (size_t countX = 0; countX < PalaceCountX; countX++) {
            printf(" -------");
        }
        printf("\n");
        for (size_t row = 0; row < Rows; row++) {
            assert(board.size() >= Rows);
            printf(" | ");
            for (size_t col = 0; col < Cols; col++) {
                assert(board[row].size() >= Cols);
                char val = board[row][col];
                if (val == ' ' || val == '0' || val == '-')
                    printf(". ");
                else
                    printf("%c ", val);
                if ((col % PalaceCols) == (PalaceCols - 1))
                    printf("| ");
            }
            printf("\n");
            if ((row % PalaceRows) == (PalaceRows - 1)) {
                // printf("  ------- ------- -------\n");
                printf(" ");
                for (size_t countX = 0; countX < PalaceCountX; countX++) {
                    printf(" -------");
                }
                printf("\n");
            }
        }
        printf("\n");
    }

    static void display_answers(std::vector<std::vector<std::vector<char>>> & answers) {
        printf("Total answers: %d\n\n", (int)answers.size());
        int i = 0;
        for (auto answer : answers) {
            BasicSudokuHelper::display_board(answer, false, i);
            i++;
        }
    }
};

typedef BasicSudokuHelper<3, 3, 3, 3, 1, 9> SudokuHelper;

} // namespace Problem_37
} // namespace LeetCode

#endif // LEETCODE_SUDOKU_SOLVER_H
