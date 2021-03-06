
#ifndef LEETCODE_SUDOKU_SOLVER_V8_H
#define LEETCODE_SUDOKU_SOLVER_V8_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <vector>
#include <list>
#include <bitset>

#include "SudokuSolver.h"
#include "BitUtils.h"
#include "StopWatch.h"

#define V8_SEARCH_ALL_ANSWERS   0

#ifdef NDEBUG
#define V8_USE_MOVE_PATH        0
#else
#define V8_USE_MOVE_PATH        1
#endif

#define V8_USE_ROW_COL_MASK     1

namespace LeetCode {
namespace Problem_37 {
namespace v8 {

#if V8_SEARCH_ALL_ANSWERS
static const bool kSearchAllAnswers = true;
#else
static const bool kSearchAllAnswers = false;
#endif

static const bool kAllowFindInvalidIndex = false;

// End game empties threshold
static const size_t kEndGameEmptiesThreshold = 42;

// Print debug trace ?
static const bool kEnableDebugTrace = false;

struct PalaceNeighbor {
    int rows[2];
    int cols[2];
};

static const PalaceNeighbor sPalaceNeighbor[9] = {
    // Palace # 0
    {
        { 1, 2 },
        { 3, 6 }
    },

    // Palace # 1
    {
        { 0, 2 },
        { 4, 7 }
    },

    // Palace # 2
    {
        { 0, 1 },
        { 5, 8 }
    },

    // Palace # 3
    {
        { 4, 5 },
        { 0, 6 }
    },

    // Palace # 4
    {
        { 3, 5 },
        { 1, 7 }
    },

    // Palace # 5
    {
        { 3, 4 },
        { 2, 8 }
    },

    // Palace # 6
    {
        { 7, 8 },
        { 0, 3 }
    },

    // Palace # 7
    {
        { 6, 8 },
        { 1, 4 }
    },

    // Palace # 8
    {
        { 6, 7 },
        { 2, 5 }
    }
};

template <typename T, size_t Capacity>
class SmallFixedDualList {
public:
    typedef T                               value_type;
    typedef SmallFixedDualList<T, Capacity> this_type;

    struct Node {
        int        prev;
        int        next;
        value_type value;

        Node() = default;
        Node(int prev, int next) : prev(prev), next(next) {
        }
        Node(const Node & src) = default;
        ~Node() = default;
    };

    typedef Node node_type;

    static const size_t kCapacity = Capacity + 1;

private:
    size_t           size_;
    size_t           capacity_;
    node_type        list_[kCapacity];

    void init() {
        this->list_[0].prev = 0;
        this->list_[0].next = 1;
    }

public:
    SmallFixedDualList() : size_(1), capacity_(2) {
        this->init();
    }
    ~SmallFixedDualList() {}

    int begin() const { return this->list_[0].next; }
    int end() const   { return 0; }

    int next(int index) const {
        return this->list_[index].next;
    }

    size_t size() const { return this->size_; }
    size_t capacity() const { return this->capacity_; }
    size_t max_capacity() const { return this_type::kCapacity; }

    void init_all() {
        for (int i = 0; i < this_type::kCapacity; i++) {
            this->list_[i].prev = i - 1;
            this->list_[i].next = i + 1;
        }
        this->list_[0] = this_type::kCapacity - 1;
        this->list_[this_type::kCapacity - 1].next = 0;
    }

    void finalize() {
        this->list_[0].prev = (int)(this->size_ - 1);
        this->list_[this->size_ - 1].next = 0;
        this->capacity_ = this->size_ + 1;
    }

    int find(const value_type & key) {
        int index = this->list_[0].next;
        while (index != 0) {
            const value_type & value = this->list_[index].value;
            if (key == value) {
                return index;
            }
            index = this->list_[index].next;
        }
        return index;
    }

    template <typename ... Args>
    void insert(int index, Args && ... args) {
        assert(index > 0);
        assert(index < this->max_capacity());
        assert(this->size_ < this->max_capacity());
        this->list_[index].prev = index - 1;
        this->list_[index].next = index + 1;
        new (&(this->list_[index].value)) value_type(std::forward<Args>(args)...);
        this->size_++;
    }

    void remove(int index) {
        assert(index > 0);
        assert(index < this->capacity());
        assert(this->size_ < this->capacity());
        assert(this->size_ < this->max_capacity());
        node_type & node = this->list_[index];
        this->list_[node.prev].next = node.next;
        this->list_[node.next].prev = node.prev;
        assert(this->size_ > 0);
        this->size_--;
    }

    void push_front(int index) {
        assert(index > 0);
        assert(index < this->capacity());
        assert(this->size_ < this->capacity());
        assert(this->size_ < this->max_capacity());
        this->list_[index].prev = 0;
        this->list_[index].next = this->list_[0].next;
        this->list_[this->list_[0].next].prev = index;
        this->list_[0].next = index;
        this->size_++;
    }

    void restore(int index) {
        assert(index > 0);
        assert(index < this->capacity());
        assert(this->size_ < this->capacity());
        assert(this->size_ < this->max_capacity());
        node_type & node = this->list_[index];
        this->list_[node.next].prev = index;
        this->list_[node.prev].next = index;
        this->size_++;
    }

    value_type & operator [] (int index) {
        assert(index < this->capacity());
        assert(index < this->max_capacity());
        return this->list_[index].value;
    };

    const value_type & operator [] (int index) const {
        assert(index < this->capacity());
        assert(index < this->max_capacity());
        return this->list_[index].value;
    };
};

class Solution {
public:
    static const size_t Rows = SudokuHelper::Rows;
    static const size_t Cols = SudokuHelper::Cols;
    static const size_t Palaces = SudokuHelper::Palaces;
    static const size_t Numbers = SudokuHelper::Numbers;

    static size_t recur_counter;
    static size_t end_recur_counter;

    struct PosInfo {
        uint8_t row;
        uint8_t col;
        uint8_t palace;
        uint8_t reserve;

        PosInfo() = default;
        PosInfo(size_t row, size_t col, bool is_fast_ctor)
            : row((uint8_t)row), col((uint8_t)col) {
        }
        PosInfo(size_t row, size_t col)
            : row((uint8_t)row), col((uint8_t)col) {
            this->palace = (uint8_t)(row / 3 * 3 + col / 3);
        }
        ~PosInfo() = default;

        bool operator == (const PosInfo & rhs) const {
            return ((this->row == rhs.row) && (this->col == rhs.col));
        }
    };

    struct NumInfo {
        uint8_t palace;
        uint8_t num;
        uint8_t palace_row;
        uint8_t palace_col;

        NumInfo() = default;
        NumInfo(size_t palace, size_t num, bool is_fast_ctor)
            : palace((uint8_t)palace), num((uint8_t)num) {
        }
        NumInfo(size_t palace, size_t num)
            : palace((uint8_t)palace), num((uint8_t)num) {
            this->palace_row = (uint8_t)((palace / 3) * 3);
            this->palace_col = (uint8_t)((palace % 3) * 3);
        }
        ~NumInfo() = default;

        bool operator == (const NumInfo & rhs) const {
            return ((this->palace == rhs.palace) && (this->num == rhs.num));
        }
    };

    struct MoveInfo {
        uint32_t row;
        uint32_t col;
        uint32_t num;

        MoveInfo() = default;
        MoveInfo(size_t row, size_t col, size_t num)
            : row((uint32_t)row), col((uint32_t)col), num((uint32_t)num) {}
        ~MoveInfo() = default;
    };

    enum MoveType {
        ByPalaceNumber,
        ByLocation,
        Unknown
    };

private:
    SmallBitMatrix2<9, 9>    rows;          // [row][num]
    SmallBitMatrix2<9, 9>    cols;          // [col][num]
    SmallBitMatrix2<9, 9>    palaces;       // [palace][num]
    std::bitset<81>          cell_filled;   // [row * 9 + col]
    SmallBitMatrix2<81, 9>   nums_usable;   // [row * 9 + col][num]

    //SmallBitMatrix3<9, 9, 3> palace_rows;   // [palace][num][row]
    //SmallBitMatrix3<9, 9, 3> palace_cols;   // [palace][num][col]
    SmallBitMatrix3<9, 9, 9> palace_nums;   // [palace][num][pos]

    //SmallBitMatrix2<3, 9>   palace_row_mask;
    //SmallBitMatrix2<3, 9>   palace_col_mask;

    SmallBitMatrix2<3, 9>   palace_row_rmask;
    SmallBitMatrix2<3, 9>   palace_col_rmask;

    uint8_t row_col_index[88];
    uint8_t palace_num_index[88];

    SmallBitMatrix3<81, 5, 9> save_palace_nums;

#if V8_USE_MOVE_PATH
    std::vector<MoveInfo> move_path;
#endif

    std::vector<std::vector<std::vector<char>>> answers;

public:
    Solution() {
        size_t mask;
#if 0
        // 0x07 = ob000000111 (2)
        mask = size_t(0x07);
        for (size_t row = 0; row < 3; row++) {
            this->palace_row_mask[row] = mask;
            mask <<= 3;
        }

        // 0x49 = 0b001001001 (2)
        mask = size_t(0x49);
        for (size_t col = 0; col < 3; col++) {
            this->palace_col_mask[col] = mask;
            mask <<= 1;
        }
#endif
        // Reverse mask, 0x07 = ob000000111 (2)
        mask = size_t(0x07);
        for (size_t row = 0; row < 3; row++) {
            this->palace_row_rmask[row] = ~mask;
            mask <<= 3;
        }

        // Reverse mask, 0x49 = 0b001001001 (2)
        mask = size_t(0x49);
        for (size_t col = 0; col < 3; col++) {
            this->palace_col_rmask[col] = ~mask;
            mask <<= 1;
        }
    }
    ~Solution() = default;

    void debug_trace(const char * fmt, ...) {
        if (kEnableDebugTrace) {
            va_list args;
            va_start(args, fmt);
            ::vprintf(fmt, args);
            va_end(args);
        }
    }

    int getNextFillCell(SmallFixedDualList<PosInfo, 82> & valid_moves) {
        assert(valid_moves.size() > 1);

        int min_index = -1;

        // Find the position that unique number or minimum numbers.
        size_t minUsable = size_t(-1);
        for (int index = valid_moves.begin(); index != valid_moves.end(); index = valid_moves.next(index)) {
            size_t row = valid_moves[index].row;
            size_t col = valid_moves[index].col;
            size_t numUsable = this->nums_usable[row * 9 + col].count();
            if (numUsable < minUsable) {
                if (numUsable == 0) {
                    return -1;
                }
                else if (numUsable == 1) {
                    return index;
                }
                minUsable = numUsable;
                min_index = index;
            }
        }

        return min_index;
    }

    int getNextFillCell(SmallFixedDualList<NumInfo, 82> & valid_nums,
                        SmallFixedDualList<PosInfo, 82> & valid_moves,
                        int & out_move_type) {
        assert(valid_nums.size() > 1);
        assert(valid_moves.size() > 1);

        int move_type = MoveType::Unknown;
        int min_index = -1;

        // Find the number that unique position or minimum positions to each palace.
        size_t minPosition = size_t(-1);
        for (int index = valid_nums.begin(); index != valid_nums.end(); index = valid_nums.next(index)) {
            size_t palace = valid_nums[index].palace;
            size_t num    = valid_nums[index].num;
            size_t numPosition = this->palace_nums[palace][num].count();
            if (numPosition < minPosition) {
                if (numPosition == 0) {
                    return -1;
                }
                else if (numPosition == 1) {
                    out_move_type = MoveType::ByPalaceNumber;
                    return index;
                }
                minPosition = numPosition;
                min_index = index;
            }
        }

        if (min_index > 0) {
            move_type = MoveType::ByPalaceNumber;
        }

        // Find the position that unique number or minimum numbers.
        size_t minUsable = minPosition;
        for (int index = valid_moves.begin(); index != valid_moves.end(); index = valid_moves.next(index)) {
            size_t row = valid_moves[index].row;
            size_t col = valid_moves[index].col;
            size_t numUsable = this->nums_usable[row * 9 + col].count();
            if (numUsable < minUsable) {
                if (numUsable == 0) {
                    return -1;
                }
                else if (numUsable == 1) {
                    out_move_type = MoveType::ByLocation;
                    return index;
                }
                minUsable = numUsable;
                min_index = index;
                move_type = MoveType::ByLocation;
            }
        }

        out_move_type = move_type;
        return min_index;
    }

    std::bitset<9> getUsable(size_t row, size_t col) {
        size_t palace = row / 3 * 3 + col / 3;
        return ~(this->rows[row] | this->cols[col] | this->palaces[palace]);
    }

    std::bitset<9> getUsable(size_t row, size_t col, size_t palace) {
        return ~(this->rows[row] | this->cols[col] | this->palaces[palace]);
    }

    std::bitset<9> getUsableStrict(size_t row, size_t col) {
        if (!this->cell_filled.test(row * 9 + col)) {
            size_t palace = row / 3 * 3 + col / 3;
            return ~(this->rows[row] | this->cols[col] | this->palaces[palace]);
        }
        else {
            return 0;
        }
    }

    std::bitset<9> getUsableStrict(size_t row, size_t col, size_t palace) {
        if (!this->cell_filled.test(row * 9 + col))
            return ~(this->rows[row] | this->cols[col] | this->palaces[palace]);
        else
            return 0;
    }

    template <bool isEndGame>
    void updateUsable(size_t row, size_t col, size_t num) {
        size_t cell_y = row * 9;
        for (size_t x = 0; x < Cols; x++) {
            if (false || (x != col)) {
                this->nums_usable[cell_y + x].reset(num);
            }
        }

        size_t cell_x = col;
        for (size_t y = 0; y < Rows; y++) {
            if (y != row) {
                this->nums_usable[y * 9 + cell_x].reset(num);
            }
        }

        size_t palace_row_idx = row / 3;
        size_t palace_col_idx = col / 3;
        if (!isEndGame) {
            size_t palace = palace_row_idx * 3 + palace_col_idx;
            //this->palace_rows[palace][num].reset();
            //this->palace_cols[palace][num].reset();
            this->palace_nums[palace][num].reset();

            //std::bitset<9> num_bits;
            //size_t num_mask;
            size_t palace_row = row % 3;
            size_t palace_col = col % 3;
            size_t palace_pos = palace_row * 3 + palace_col;
            SmallBitMatrix2<9, 9> & numbers_pos = this->palace_nums[palace];
            for (size_t _num = 0; _num < SudokuHelper::Numbers; _num++) {
                if (!this->palaces[palace].test(_num)) {
                    if (numbers_pos[_num].test(palace_pos)) {
#if 0
                        num_bits = this->palace_nums[palace][_num] & this->palace_row_mask[palace_row];
                        num_mask = size_t(1) << palace_pos;
                        if (num_bits == num_mask) {
                            this->palace_rows[palace][_num].reset(palace_row);
                        }
                        num_bits = this->palace_nums[palace][_num] & this->palace_col_mask[palace_col];
                        if (num_bits == num_mask) {
                            this->palace_cols[palace][_num].reset(palace_col);
                        }
#endif
                        numbers_pos[_num].reset(palace_pos);
                    }
                }
            }

            for (size_t idx = 0; idx < 2; idx++) {
                size_t palace_row_id = sPalaceNeighbor[palace].rows[idx];
                if (!this->palaces[palace_row_id].test(num)) {
#if 0
                    if (this->palace_nums[palace_row_id][num].test(palace_row * 3 + 0)) {
                        num_bits = this->palace_nums[palace_row_id][num] & this->palace_col_mask[palace_col];
                        num_mask = size_t(1) << (palace_row * 3 + 0);
                        if (num_bits == num_mask) {
                            this->palace_cols[palace_row_id][num].reset(0);
                        }
                    }
                    if (this->palace_nums[palace_row_id][num].test(palace_row * 3 + 1)) {
                        num_bits = this->palace_nums[palace_row_id][num] & this->palace_col_mask[palace_col];
                        num_mask = size_t(1) << (palace_row * 3 + 1);
                        if (num_bits == num_mask) {
                            this->palace_cols[palace_row_id][num].reset(1);
                        }
                    }
                    if (this->palace_nums[palace_row_id][num].test(palace_row * 3 + 2)) {
                        num_bits = this->palace_nums[palace_row_id][num] & this->palace_col_mask[palace_col];
                        num_mask = size_t(1) << (palace_row * 3 + 2);
                        if (num_bits == num_mask) {
                            this->palace_cols[palace_row_id][num].reset(2);
                        }
                    }

                    this->palace_rows[palace_row_id][num].reset(palace_row);
#endif
#if V8_USE_ROW_COL_MASK
                    this->palace_nums[palace_row_id][num] &= this->palace_row_rmask[palace_row];
#else
                    this->palace_nums[palace_row_id][num].reset(palace_row * 3 + 0);
                    this->palace_nums[palace_row_id][num].reset(palace_row * 3 + 1);
                    this->palace_nums[palace_row_id][num].reset(palace_row * 3 + 2);
#endif
                }

                size_t palace_col_id = sPalaceNeighbor[palace].cols[idx];
                if (!this->palaces[palace_col_id].test(num)) {
#if 0
                    if (this->palace_nums[palace_col_id][num].test(0 * 3 + palace_col)) {
                        num_bits = this->palace_nums[palace_col_id][num] & this->palace_row_mask[palace_row];
                        num_mask = size_t(1) << (0 * 3 + palace_col);
                        if (num_bits == num_mask) {
                            this->palace_rows[palace_col_id][num].reset(0);
                        }
                    }
                    if (this->palace_nums[palace_col_id][num].test(1 * 3 + palace_col)) {
                        num_bits = this->palace_nums[palace_col_id][num] & this->palace_row_mask[palace_row];
                        num_mask = size_t(1) << (1 * 3 + palace_col);
                        if (num_bits == num_mask) {
                            this->palace_rows[palace_col_id][num].reset(1);
                        }
                    }
                    if (this->palace_nums[palace_col_id][num].test(2 * 3 + palace_col)) {
                        num_bits = this->palace_nums[palace_col_id][num] & this->palace_row_mask[palace_row];
                        num_mask = size_t(1) << (2 * 3 + palace_col);
                        if (num_bits == num_mask) {
                            this->palace_rows[palace_col_id][num].reset(2);
                        }
                    }

                    this->palace_cols[palace_col_id][num].reset(palace_col);
#endif
#if V8_USE_ROW_COL_MASK
                    this->palace_nums[palace_col_id][num] &= this->palace_col_rmask[palace_col];
#else
                    this->palace_nums[palace_col_id][num].reset(0 * 3 + palace_col);
                    this->palace_nums[palace_col_id][num].reset(1 * 3 + palace_col);
                    this->palace_nums[palace_col_id][num].reset(2 * 3 + palace_col);
#endif
                }
            }
        }

        size_t cell = cell_y + cell_x;
        size_t pos = palace_row_idx * 3 * 9 + palace_col_idx * 3;
        for (size_t y = 0; y < (Rows / 3); y++) {
            for (size_t x = 0; x < (Cols / 3); x++) {
                if (pos != cell) {
                    this->nums_usable[pos].reset(num);
                }
                pos++;
            }
            pos += (9 - 3);
        }

        //this->nums_usable[cell].reset();
    }

    template <bool isUndo = true>
    void updateUndoUsable(size_t row, size_t col, size_t _num) {
        size_t cell_y = row * 9;
        size_t palace_row = row / 3 * 3;
        for (size_t x = 0; x < Cols; x++) {
            if (isUndo || (x != col)) {
                if (!this->cell_filled.test(row * 9 + x)) {
                    size_t palace = palace_row + x / 3;
                    this->nums_usable[cell_y + x] = getUsable(row, x, palace);
                }
            }
        }

        size_t cell_x = col;
        size_t palace_col = col / 3;
        for (size_t y = 0; y < Rows; y++) {
            if (y != row) {
                if (!this->cell_filled.test(y * 9 + col)) {
                    size_t palace = y / 3 * 3 + palace_col;
                    this->nums_usable[y * 9 + cell_x] = getUsable(y, col, palace);
                }
            }
        }

        size_t palace = palace_row + palace_col;
        size_t cell = cell_y + cell_x;
        palace_col *= 3;
        size_t pos = palace_row * 9 + palace_col;
        for (size_t y = 0; y < (Rows / 3); y++) {
            for (size_t x = 0; x < (Cols / 3); x++) {
                if (pos != cell) {
                    if (!this->cell_filled.test((palace_row + y) * 9 + (palace_col + x))) {
                        this->nums_usable[pos] = getUsable(palace_row + y, palace_col + x, palace);
                    }
                }
                pos++;
            }
            pos += (9 - 3);
        }

#if 0
        palace_row = (row % 3);
        palace_col = (col % 3);
        size_t palace_pos = palace_row * 3 + palace_col;
        this->palace_rows[palace][_num].set(palace_row);
        this->palace_cols[palace][_num].set(palace_col);
        this->palace_nums[palace][_num].set(palace_pos);

        for (int index = valid_nums.begin(); index != valid_nums.end(); index = valid_nums.next(index)) {
            size_t palace = valid_nums[index].palace;
            size_t num    = valid_nums[index].num;
            const std::bitset<9> & validPos = this->palace_nums[palace][num];
            for (size_t pos = 0; pos < validPos.size(); pos++) {
                // Get usable position
                if (validPos.test(pos)) {
                    row = valid_nums[index].palace_row + (pos / 3);
                    col = valid_nums[index].palace_col + (pos % 3);
                    std::bitset<9> numsUsable = getUsable(row, col, palace);

                    // Get usable positions each number in the same palace.
                    size_t palace_row = (row % 3);
                    size_t palace_col = (col % 3);
                    size_t palace_pos = palace_row * 3 + palace_col;
                    bool isUsable = numsUsable.test(num);
                    if (isUsable) {
                        this->palace_rows[palace][num] |= size_t(1) << (palace_row);
                        this->palace_cols[palace][num] |= size_t(1) << (palace_col);
                        this->palace_nums[palace][num].set(palace_pos);
                    }
                }
            }
        }
#endif
    }

    void fillNum(size_t row, size_t col, size_t num) {
        size_t palace = row / 3 * 3 + col / 3;
        this->rows[row].set(num);
        this->cols[col].set(num);
        this->palaces[palace].set(num);
        this->cell_filled.set(row * 9 + col);
    }

    template <bool isEndGame = false>
    void doFillNum(size_t row, size_t col, size_t num) {
        size_t palace = row / 3 * 3 + col / 3;
        this->rows[row].set(num);
        this->cols[col].set(num);
        this->palaces[palace].set(num);
        //this->cell_filled.set(row * 9 + col);
        updateUsable<isEndGame>(row, col, num);
#if V8_USE_MOVE_PATH
        this->move_path.push_back(MoveInfo((uint32_t)row, (uint32_t)col, (uint32_t)(num + 1)));
#endif
    }

    template <bool isEndGame = false>
    void undoFillNum(size_t row, size_t col, size_t num) {
        size_t palace = row / 3 * 3 + col / 3;
        this->rows[row].reset(num);
        this->cols[col].reset(num);
        this->palaces[palace].reset(num);
        //this->cell_filled.reset(row * 9 + col);
        updateUndoUsable<false>(row, col, num);
#if V8_USE_MOVE_PATH
        this->move_path.pop_back();
#endif
    }

    void savePalaceNumsState(size_t depth, size_t palace, size_t num) {
        SmallBitMatrix2<5, 9> & save_status = this->save_palace_nums[depth];
        save_status[0] = this->palace_nums[palace][num];

        const PalaceNeighbor & neighbor = sPalaceNeighbor[palace];
        save_status[1] = this->palace_nums[neighbor.rows[0]][num];
        save_status[2] = this->palace_nums[neighbor.rows[1]][num];
        save_status[3] = this->palace_nums[neighbor.cols[0]][num];
        save_status[4] = this->palace_nums[neighbor.cols[1]][num];
    }

    void restorePalaceNumsState(size_t depth, size_t row, size_t col,
                                 size_t palace, size_t palace_pos, size_t num) {
        const SmallBitMatrix2<5, 9> & save_status = this->save_palace_nums[depth];
        this->palace_nums[palace][num] = save_status[0];

        const PalaceNeighbor & neighbor = sPalaceNeighbor[palace];
        this->palace_nums[neighbor.rows[0]][num] = save_status[1];
        this->palace_nums[neighbor.rows[1]][num] = save_status[2];
        this->palace_nums[neighbor.cols[0]][num] = save_status[3];
        this->palace_nums[neighbor.cols[1]][num] = save_status[4];

        std::bitset<9> numsUsable = getUsable(row, col, palace);
        SmallBitMatrix2<9, 9> & numbers_pos = this->palace_nums[palace];
        for (size_t _num = 0; _num < SudokuHelper::Numbers; _num++) {
            if (!this->palaces[palace].test(_num)) {
                numbers_pos[_num][palace_pos] = numsUsable.test(_num);
            }
        }
    }

    void restorePalaceNumsState(size_t depth,
                                 size_t row, size_t col,
                                 size_t palace, size_t num) {
        size_t palace_pos = (row % 3) * 3 + (col % 3);
        restorePalaceNumsState(depth, row, col, palace, palace_pos, num);
    }

    template <bool NeedSearchAllAnswers = false>
    bool solve_end(size_t depth, std::vector<std::vector<char>> & board,
                   SmallFixedDualList<PosInfo, 82> & valid_moves) {
        if (valid_moves.size() <= 1) {
            if (NeedSearchAllAnswers)
                this->answers.push_back(board);
            else
                return true;
        }

        end_recur_counter++;

        int move_idx = getNextFillCell(valid_moves);
        if (move_idx > 0) {
            debug_trace(">>>> EndGame [depth = %d], valid_moves.remove(move_idx = %d);\n\n",
                        (int)depth, move_idx);

            size_t row = valid_moves[move_idx].row;
            size_t col = valid_moves[move_idx].col;
            valid_moves.remove(move_idx);

            size_t cell = row * 9 + col;
            std::bitset<9> validNums = this->nums_usable[cell];
            size_t num_count = validNums.count();
            assert(num_count != 0);
            if (num_count == 1) {
                this->nums_usable[cell].reset();
                this->cell_filled.set(cell);

                size_t numBits = validNums.to_ullong();
                size_t num = jstd::BitUtils::bsf(numBits);
                doFillNum<true>(row, col, num);
                debug_trace(">>   row: %d, col: %d, num: %d\n\n", (int)row, (int)col, (int)num);

                board[row][col] = (char)(num + '1');

                if (solve_end<NeedSearchAllAnswers>(depth + 1, board, valid_moves)) {
                    if (!NeedSearchAllAnswers) {
                        return true;
                    }
                }

                board[row][col] = '.';
                undoFillNum<true>(row, col, num);

                this->cell_filled.reset(cell);
                this->nums_usable[cell] = validNums;
            }
            else {
                this->nums_usable[cell].reset();
                this->cell_filled.set(cell);
                size_t count = 0;
                for (size_t num = 0; num < validNums.size(); num++) {
                    // Get usable numbers
                    if (validNums.test(num)) {
                        doFillNum<true>(row, col, num);
                        debug_trace(">>   row: %d, col: %d, num: %d\n\n", (int)row, (int)col, (int)num);

                        board[row][col] = (char)(num + '1');

                        if (solve_end<NeedSearchAllAnswers>(depth + 1, board, valid_moves)) {
                            if (!NeedSearchAllAnswers) {
                                return true;
                            }
                        }

                        board[row][col] = '.';
                        undoFillNum<true>(row, col, num);

                        count++;
                        if (count >= num_count)
                            break;
                    }
                }
                this->cell_filled.reset(cell);
                this->nums_usable[cell] = validNums;
            }

            valid_moves.push_front(move_idx);
            debug_trace(">>>> EndGame [depth = %d] backtracking.\n\n", (int)depth);
        }

        return false;
    }

    template <bool NeedSearchAllAnswers = false>
    bool solve(size_t depth, std::vector<std::vector<char>> & board,
               SmallFixedDualList<NumInfo, 82> & valid_nums,
               SmallFixedDualList<PosInfo, 82> & valid_moves) {
        if (valid_moves.size() <= 1) {
            if (NeedSearchAllAnswers)
                this->answers.push_back(board);
            else
                return true;
        }

        recur_counter++;

        int move_type;
        int move_idx = getNextFillCell(valid_nums, valid_moves, move_type);
        if (move_idx > 0) {
            if (move_type == MoveType::ByPalaceNumber) {
                debug_trace(">>>> [depth = %d] valid_nums .remove(move_idx  = %d);\n\n",
                            (int)depth, move_idx);

                size_t palace = valid_nums[move_idx].palace;
                size_t num    = valid_nums[move_idx].num;
                valid_nums.remove(move_idx);

                std::bitset<9> validPos = this->palace_nums[palace][num];
                this->palace_nums[palace][num].reset();

                size_t pos_count = validPos.count();
                size_t count = 0;
                assert(validPos.count() != 0);
                for (size_t pos = 0; pos < validPos.size(); pos++) {
                    // Get usable position
                    if (validPos.test(pos)) {
                        size_t row = valid_nums[move_idx].palace_row + (pos / 3);
                        size_t col = valid_nums[move_idx].palace_col + (pos % 3);
                        size_t cell = row * 9 + col;
                        this->cell_filled.set(cell);

                        // Save current palace bitset state
                        savePalaceNumsState(depth, palace, num);

                        doFillNum<false>(row, col, num);
                        this->nums_usable[cell].reset();

                        int pos_index = this->row_col_index[row * 9 + col];
                        assert(pos_index > 0);
                        if (!kAllowFindInvalidIndex || (pos_index > 0)) {
                            valid_moves.remove(pos_index);
                            debug_trace(">>   valid_moves.remove(pos_index = %d); (*)\n\n", pos_index);
                        }

                        debug_trace(">>   palace: %d, pos: %d, num: %d\n", (int)palace, (int)pos, (int)num);
                        debug_trace(">>   row: %d, col: %d, num: %d\n\n", (int)row, (int)col, (int)num);

                        board[row][col] = (char)(num + '1');

                        if (kAllowFindInvalidIndex && (pos_index <= 0)) {
                            return true;
                        }

                        // kEndGameEmptiesThreshold = 40
                        if (valid_moves.size() > (kEndGameEmptiesThreshold + 1)) {
                            if (solve<NeedSearchAllAnswers>(depth + 1, board, valid_nums, valid_moves)) {
                                if (!NeedSearchAllAnswers) {
                                    return true;
                                }
                            }
                        }
                        else {
                            if (solve_end<NeedSearchAllAnswers>(depth + 1, board, valid_moves)) {
                                if (!NeedSearchAllAnswers) {
                                    return true;
                                }
                            }
                        }

                        board[row][col] = '.';
                        if (!kAllowFindInvalidIndex || (pos_index > 0)) {
                            valid_moves.push_front(pos_index);
                        }

                        undoFillNum<false>(row, col, num);

                        restorePalaceNumsState(depth, row, col, palace, pos, num);

                        this->nums_usable[cell] = getUsable(row, col, palace);
                        this->cell_filled.reset(cell);

                        count++;
                        if (count >= pos_count)
                            break;
                    }
                }

                this->palace_nums[palace][num] = validPos;
                valid_nums.push_front(move_idx);
                debug_trace(">>>> [depth = %d], backtracking.\n\n", (int)depth);
            }
            else {
                assert(move_type == MoveType::ByLocation);
                debug_trace(">>>> [depth = %d] valid_moves.remove(move_idx = %d);\n\n",
                            (int)depth, move_idx);

                size_t row = valid_moves[move_idx].row;
                size_t col = valid_moves[move_idx].col;
                size_t palace = valid_moves[move_idx].palace;
                valid_moves.remove(move_idx);

                size_t cell = row * 9 + col;
                std::bitset<9> validNums = this->nums_usable[cell];
                this->nums_usable[cell].reset();
                this->cell_filled.set(cell);

                size_t num_count = validNums.count();
                size_t count = 0;
                assert(validNums.count() != 0);
                for (size_t num = 0; num < validNums.size(); num++) {
                    // Get usable numbers
                    if (validNums.test(num)) {
                        // Save current palace bitset state
                        savePalaceNumsState(depth, palace, num);

                        doFillNum<false>(row, col, num);

                        int num_index = this->palace_num_index[palace * 9 + num];
                        assert(num_index > 0);
                        if (!kAllowFindInvalidIndex || (num_index > 0)) {
                            valid_nums.remove(num_index);
                            debug_trace(">>   valid_nums .remove(num_index = %d);\n\n", num_index);
                        }

                        debug_trace(">>   palace: %d, pos: %d, num: %d\n", (int)palace, (int)((row % 3) * 3 + (col % 3)), (int)num);
                        debug_trace(">>   row: %d, col: %d, num: %d\n\n", (int)row, (int)col, (int)num);

                        board[row][col] = (char)(num + '1');

                        if (kAllowFindInvalidIndex && (num_index <= 0)) {
                            return true;
                        }

                        // kEndGameEmptiesThreshold = 40
                        if (valid_moves.size() > (kEndGameEmptiesThreshold + 1)) {
                            if (solve<NeedSearchAllAnswers>(depth + 1, board, valid_nums, valid_moves)) {
                                if (!NeedSearchAllAnswers) {
                                    return true;
                                }
                            }
                        }
                        else {
                            if (solve_end<NeedSearchAllAnswers>(depth + 1, board, valid_moves)) {
                                if (!NeedSearchAllAnswers) {
                                    return true;
                                }
                            }
                        }

                        board[row][col] = '.';
                        if (!kAllowFindInvalidIndex || (num_index > 0)) {
                            valid_nums.push_front(num_index);
                        }

                        undoFillNum<false>(row, col, num);

                        restorePalaceNumsState(depth, row, col, palace, num);

                        count++;
                        if (count >= num_count)
                            break;
                    }
                }

                this->cell_filled.reset(cell);
                this->nums_usable[cell] = validNums;

                valid_moves.push_front(move_idx);
                debug_trace(">>>> [depth = %d] backtracking.\n\n", (int)depth);
            }
        }

        return false;
    }

    double solveSudoku(std::vector<std::vector<char>> & board, bool verbose = true) {
        double elapsed_time;
        if (verbose) {
            SudokuHelper::display_board(board, true);
        }
        recur_counter = 0;
        end_recur_counter = 0;

        jtest::StopWatch sw;
        sw.start();

        SmallFixedDualList<NumInfo, 82> valid_nums;
        SmallFixedDualList<PosInfo, 82> valid_moves;

        int index = 1;
        for (size_t row = 0; row < board.size(); row++) {
            const std::vector<char> & line = board[row];
            for (size_t col = 0; col < line.size(); col++) {
                size_t pos = row * 9 + col;
                char val = line[col];
                if (val != '.') {
                    this->row_col_index[pos] = -1;
                    size_t num = val - '1';
                    this->fillNum(row, col, num);
                }
                else {
                    this->row_col_index[pos] = index;
                    valid_moves.insert(index, row, col);
                    index++;
                }
            }   
        }
        valid_moves.finalize();

        for (size_t row = 0; row < board.size(); row++) {
            const std::vector<char> & line = board[row];
            for (size_t col = 0; col < line.size(); col++) {
                char val = line[col];
                if (val == '.') {
                    // Get usable numbers each position.
                    size_t palace = row / 3 * 3 + col / 3;
                    std::bitset<9> numsUsable = getUsable(row, col, palace);
                    this->nums_usable[row * 9 + col] = numsUsable;

                    // Get usable positions each number in the same palace.
                    size_t palace_row = (row % 3);
                    size_t palace_col = (col % 3);
                    size_t palace_pos = palace_row * 3 + palace_col;
                    for (size_t num = 0; num < SudokuHelper::Numbers; num++) {
                        if (!this->palaces[palace].test(num)) {
                            bool isUsable = numsUsable.test(num);
                            if (isUsable) {
                                //this->palace_rows[palace][num] |= size_t(1) << (palace_row);
                                //this->palace_cols[palace][num] |= size_t(1) << (palace_col);
                                this->palace_nums[palace][num].set(palace_pos);
                            }
                        }
                    }
                }
            }
        }

Find_Next_Step:
        // Find and fill the position that unique number.
        for (int index = valid_moves.begin(); index != valid_moves.end(); index = valid_moves.next(index)) {
            size_t row = valid_moves[index].row;
            size_t col = valid_moves[index].col;
            size_t num_count = this->nums_usable[row * 9 + col].count();
            if (num_count == 1) {
                for (size_t num = 0; num < SudokuHelper::Numbers; num++) {
                    if (this->nums_usable[row * 9 + col].test(num)) {
                        doFillNum<false>(row, col, num);
                        board[row][col] = (char)(num + '1');
                        valid_moves.remove(index);
                        //size_t palace = row / 3 * 3 + col / 3;
                        //int num_index = this->palace_num_index[palace * 9 + num];
                        //assert(num_index > 0);
                        //valid_nums.remove(num_index);
                        goto Find_Next_Step;
                    }
                }
            }
        }
        
        // Find and fill the number that unique position to each palace.
        for (size_t palace = 0; palace < SudokuHelper::Palaces; palace++) {
            for (size_t num = 0; num < SudokuHelper::Numbers; num++) {
                size_t pos_count = this->palace_nums[palace][num].count();
                if (pos_count == 1) {
                    for (size_t pos = 0; pos < SudokuHelper::Numbers; pos++) {
                        if (this->palace_nums[palace][num].test(pos)) {
                            size_t palace_row = palace / 3;
                            size_t palace_col = palace % 3;
                            size_t row = palace_row * 3 + pos / 3;
                            size_t col = palace_col * 3 + pos % 3;

                            doFillNum<false>(row, col, num);
                            board[row][col] = (char)(num + '1');

                            //int num_index = this->palace_num_index[palace * 9 + num];
                            //assert(num_index > 0);
                            //valid_nums.remove(num_index);

                            int move_index = this->row_col_index[row * 9 + col];
                            assert (move_index > 0);
                            valid_moves.remove(move_index);
                            goto Find_Next_Step;
                        }
                    }
                }
            }
        }

        int num_index = 1;
        for (size_t palace = 0; palace < SudokuHelper::Palaces; palace++) {
            for (size_t num = 0; num < SudokuHelper::Numbers; num++) {
                size_t palace_num = palace * 9 + num;
                size_t pos_count = this->palace_nums[palace][num].count();
                if (pos_count > 0) {
                    this->palace_num_index[palace_num] = num_index;
                    valid_nums.insert(num_index, palace, num);
                    num_index++;
                }
                else {
                    this->palace_num_index[palace_num] = -1;
                }
            }
        }
        valid_nums.finalize();

        assert(valid_moves.size() == valid_nums.size());

        this->solve<kSearchAllAnswers>(0, board, valid_nums, valid_moves);

        sw.stop();
        elapsed_time = sw.getElapsedMillisec();

        if (verbose) {
            if (kSearchAllAnswers)
                SudokuHelper::display_answers(this->answers);
            else
                SudokuHelper::display_board(board);

            printf("Elapsed time: %0.3f ms\n\n"
                   "recur_counter: %u, end_recur_counter: %u\n\n",
                   sw.getElapsedMillisec(), (uint32_t)recur_counter,
                   (uint32_t)end_recur_counter);
        }

        return elapsed_time;
    }
};

size_t Solution::recur_counter = 0;
size_t Solution::end_recur_counter = 0;

} // namespace v8
} // namespace Problem_37
} // namespace LeetCode

#endif // LEETCODE_SUDOKU_SOLVER_V8_H
