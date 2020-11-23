
#ifndef LEETCODE_SUDOKU_SOLVER_V6_H
#define LEETCODE_SUDOKU_SOLVER_V6_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <vector>
#include <list>
#include <bitset>

#include "SudokuSolver.h"
#include "StopWatch.h"

#define V6_SEARCH_ALL_STAGE     0

namespace LeetCode {
namespace Problem_37 {
namespace v6 {

#if V6_SEARCH_ALL_STAGE
static const bool kSearchAllStages = true;
#else
static const bool kSearchAllStages = false;
#endif

template <typename T, size_t Capacity>
class SmallFixedStack {
public:
    typedef T                               value_type;
    typedef SmallFixedStack<T, Capacity>    this_type;

    static const size_t kCapacity = Capacity;

private:
    int             head_;
    int             capacity_;
    value_type      data_[kCapacity];

public:
    SmallFixedStack() : head_(0), capacity_(0) {
    }
    ~SmallFixedStack() {}

    int begin() const { return this->head_; }
    int end() const   { return this->capacity_; }

    int next(int index) const {
        return (index + 1);
    }

    int has_next(int index) const {
        return (this->head_ + index < this->capacity_);
    }

    size_t head() const { return this->head_; }
    size_t tail() const { return (this->capacity_ - 1); }

    size_t size() const { return (this->capacity_ - this->head_); }
    size_t capacity() const { return this->capacity_; }
    size_t max_capacity() const { return this_type::kCapacity; }

    void finalize() {
        assert(this->capacity_ >= 1);
    }

    template <typename ... Args>
    void insert(int index, Args && ... args) {
        assert(index >= 0);
        assert(index < this->max_capacity());
        assert(this->capacity_ < this->max_capacity());
        new (&(this->data_[index])) value_type(std::forward<Args>(args)...);
        this->capacity_++;
    }

    void remove(int index) {
        assert(index >= this->head_);
        assert(index < this->capacity());
        assert(this->size() > 0);
        assert(this->size() <= this->capacity());
        assert(this->size() <= this->max_capacity());
        if (index != this->head_) {
            this->swap(this->head_, index);
        }
        this->head_++;
    }

    void restore() {
        assert(this->size() >= 0);
        assert(this->size() <= this->capacity());
        assert(this->size() <= this->max_capacity());
        this->head_--;
    }

    void swap(int index1, int index2) {
        assert(index1 != index2);
        std::swap(this->data_[index1], this->data_[index2]);
    }

    const value_type & operator [] (int index) const {
        assert(index < this->capacity());
        assert(index < this->max_capacity());
        return this->data_[index];
    };
};

class Solution {
public:
    static const size_t Rows = SudokuHelper::Rows;
    static const size_t Cols = SudokuHelper::Cols;
    static const size_t Numbers = SudokuHelper::Numbers;

    static size_t recur_counter;

    struct PosInfo {
        uint32_t row;
        uint32_t col;

        PosInfo() = default;
        PosInfo(size_t row, size_t col) : row((uint32_t)row), col((uint32_t)col) {};
        ~PosInfo() = default;
    };

private:
    SmallBitMatrix2<9, 9>  rows;
    SmallBitMatrix2<9, 9>  cols;
    SmallBitMatrix2<9, 9>  palaces;
    SmallBitMatrix2<81, 9> nums_usable;

    std::vector<std::vector<std::vector<char>>> answers;

public:
    Solution() = default;
    ~Solution() = default;

    int getNextFillCell(SmallFixedStack<PosInfo, 81> & valid_moves) {
        assert(valid_moves.size() > 0);
        size_t minUsable = size_t(-1);
        int min_index = -1;
        for (int index = valid_moves.begin(); index != valid_moves.end(); index++) {
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

    std::bitset<9> getUsable(size_t row, size_t col) {
        size_t palace = row / 3 * 3 + col / 3;
        return ~(this->rows[row] | this->cols[col] | this->palaces[palace]);
    }

    std::bitset<9> getUsable(size_t row, size_t col, size_t palace) {
        return ~(this->rows[row] | this->cols[col] | this->palaces[palace]);
    }

    void updateUsable(size_t row, size_t col, size_t num) {
        size_t cell_y = row * 9;
        for (size_t x = 0; x < Cols; x++) {
            if (x != col) {
                this->nums_usable[cell_y + x].reset(num);
            }
        }

        size_t cell_x = col;
        for (size_t y = 0; y < Rows; y++) {
            if (y != row) {
                this->nums_usable[y * 9 + cell_x].reset(num);
            }
        }

        size_t palace_row = row / 3 * 3;
        size_t palace_col = col / 3;
        size_t cell = cell_y + cell_x;
        palace_col *= 3;
        size_t pos = palace_row * 9 + palace_col;
        for (size_t y = 0; y < (Rows / 3); y++) {
            for (size_t x = 0; x < (Cols / 3); x++) {
                if (pos != cell) {
                    this->nums_usable[pos].reset(num);
                }
                pos++;
            }
            pos += (9 - 3);
        }
    }

    template <bool isUndo = true>
    void updateUndoUsable(size_t row, size_t col) {
        size_t cell_y = row * 9;
        size_t palace_row = row / 3 * 3;
        for (size_t x = 0; x < Cols; x++) {
            if (isUndo || x != col) {
                size_t palace = palace_row + x / 3;
                this->nums_usable[cell_y + x] = getUsable(row, x, palace);
            }
        }

        size_t cell_x = col;
        size_t palace_col = col / 3;
        for (size_t y = 0; y < Rows; y++) {
            if (y != row) {
                size_t palace = y / 3 * 3 + palace_col;
                this->nums_usable[y * 9 + cell_x] = getUsable(y, col, palace);
            }
        }

        size_t palace = palace_row + palace_col;
        size_t cell = cell_y + cell_x;
        palace_col *= 3;
        size_t pos = palace_row * 9 + palace_col;
        for (size_t y = 0; y < (Rows / 3); y++) {
            for (size_t x = 0; x < (Cols / 3); x++) {
                if (pos != cell) {
                    this->nums_usable[pos] = getUsable(palace_row + y, palace_col + x, palace);
                }
                pos++;
            }
            pos += (9 - 3);
        }
    }

    void fillNum(size_t row, size_t col, size_t num) {
        size_t palace = row / 3 * 3 + col / 3;
        this->rows[row].set(num);
        this->cols[col].set(num);
        this->palaces[palace].set(num);
    }

    void doFillNum(size_t row, size_t col, size_t num) {
        size_t palace = row / 3 * 3 + col / 3;
        this->rows[row].set(num);
        this->cols[col].set(num);
        this->palaces[palace].set(num);
        updateUsable(row, col, num);
    }

    void undoFillNum(size_t row, size_t col, size_t num) {
        size_t palace = row / 3 * 3 + col / 3;
        this->rows[row].reset(num);
        this->cols[col].reset(num);
        this->palaces[palace].reset(num);
        updateUndoUsable<true>(row, col);
    }

    bool solve(std::vector<std::vector<char>> & board,
               SmallFixedStack<PosInfo, 81> & valid_moves) {
        if (valid_moves.size() == 0) {
            if (kSearchAllStages)
                this->answers.push_back(board);
            else
                return true;
        }

        recur_counter++;

        int move_idx = getNextFillCell(valid_moves);
        if (move_idx >= 0) {
            size_t row = valid_moves[move_idx].row;
            size_t col = valid_moves[move_idx].col;
            valid_moves.remove(move_idx);
            const std::bitset<9> & fillNums = this->nums_usable[row * 9 + col];
            for (size_t num = 0; num < fillNums.size(); num++) {
                // Get usable numbers
                if (fillNums.test(num)) {
                    doFillNum(row, col, num);
                    board[row][col] = (char)(num + '1');

                    if (solve(board, valid_moves)) {
                        if (!kSearchAllStages)
                            return true;
                    }

                    board[row][col] = '.';
                    undoFillNum(row, col, num);
                }
            }
            valid_moves.restore();
        }

        return false;
    }

    void solveSudoku(std::vector<std::vector<char>> & board) {
        SudokuHelper::display_board(board, true);
        recur_counter = 0;

        jtest::StopWatch sw;
        sw.start();

        int index = 0;
        SmallFixedStack<PosInfo, 81> valid_moves;
        for (size_t row = 0; row < board.size(); row++) {
            const std::vector<char> & line = board[row];
            for (size_t col = 0; col < line.size(); col++) {
                char val = line[col];
                if (val != '.') {
                    size_t num = val - '1';
                    fillNum(row, col, num);
                }
                else {
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
                    this->nums_usable[row * 9 + col] = getUsable(row, col);
                }
            }
        }

        this->solve(board, valid_moves);

        sw.stop();

#if V6_SEARCH_ALL_STAGE
        SudokuHelper::display_answers(this->answers);
#else
        SudokuHelper::display_board(board);
#endif
        printf("Elapsed time: %0.3f ms, recur_counter: %u\n\n",
               sw.getElapsedMillisec(), (uint32_t)recur_counter);
    }
};

size_t Solution::recur_counter = 0;

} // namespace v6
} // namespace Problem_37
} // namespace LeetCode

#endif // LEETCODE_SUDOKU_SOLVER_V6_H
