
#ifndef LEETCODE_SUDOKU_SOLVER_V1B_H
#define LEETCODE_SUDOKU_SOLVER_V1B_H

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
#include <bitset>

#include "SudokuSolver.h"
#include "StopWatch.h"

#define V1B_SEARCH_ALL_STAGE    0
#define V1B_INSERT_MODE         1

namespace LeetCode {
namespace Problem_37 {
namespace v1b {

#if V1B_SEARCH_ALL_STAGE
static const bool kSearchAllStages = true;
#else
static const bool kSearchAllStages = false;
#endif

template <size_t Capacity>
class FixedDlxNodeList {
public:
    typedef FixedDlxNodeList<Capacity> this_type;

    int prev[Capacity];
    int next[Capacity];
    int up[Capacity];
    int down[Capacity];
    int row[Capacity];
    int col[Capacity];

private:
    size_t size_;

    static const size_t kCapacity = Capacity;

public:
    FixedDlxNodeList(size_t capacity) : size_(0) {
        this->init();
    }

    ~FixedDlxNodeList() {
        this->destroy();
    }

    size_t size() const { return this->size_; }
    size_t capacity() const { return this_type::kCapacity; }

private:
    void init() {
    }

    void destroy() {
    }
};

class DlxNodeList {
public:
    int * prev;
    int * next;
    int * up;
    int * down;
    int * row;
    int * col;

private:
    size_t size_;
    size_t capacity_;

public:
    DlxNodeList(size_t capacity)
        : prev(nullptr), next(nullptr), up(nullptr), down(nullptr),
          row(nullptr), col(nullptr), size_(0), capacity_(capacity) {
        this->init(capacity);
    }

    ~DlxNodeList() {
        this->destroy();
    }

    size_t size() const { return this->size_; }
    size_t capacity() const { return this->capacity_; }

private:
    void init(size_t capacity) {
        assert(capacity > 0);
        this->prev = new int[capacity];
        this->next = new int[capacity];
        this->up   = new int[capacity];
        this->down = new int[capacity];
        this->row  = new int[capacity];
        this->col  = new int[capacity];
    }

    void destroy() {
        if (this->prev) {
            delete[] this->prev;
            this->prev = nullptr;
        }
        if (this->next) {
            delete[] this->next;
            this->next = nullptr;
        }
        if (this->up) {
            delete[] this->up;
            this->up = nullptr;
        }
        if (this->down) {
            delete[] this->down;
            this->down = nullptr;
        }
        if (this->row) {
            delete[] this->row;
            this->row = nullptr;
        }
        if (this->col) {
            delete[] this->col;
            this->col = nullptr;
        }
    }
};

class DancingLinks {
public:
    static const size_t Rows = SudokuHelper::Rows;
    static const size_t Cols = SudokuHelper::Cols;
    static const size_t Palaces = SudokuHelper::Palaces;
    static const size_t Numbers = SudokuHelper::Numbers;

    static const size_t TotalSize = SudokuHelper::TotalSize;
    static const size_t TotalSize2 = SudokuHelper::TotalSize2;

private:    
#if 0
    DlxNodeList         list_;
#else
    FixedDlxNodeList<SudokuHelper::TotalSize * 4 + 1>
                        list_;
#endif
    std::vector<int>    col_size_;
    std::vector<int>    answer_;
    int                 last_idx_;

    size_t rows_[TotalSize + 1];
    size_t cols_[TotalSize + 1];
    size_t numbers_[TotalSize + 1];

    std::vector<std::vector<int>> answers_;

    static size_t recur_counter;

    struct StackInfo {
        int index;
        int row;

        StackInfo() : index(0), row(0) {}
        StackInfo(int index, int row) : index(index), row(row) {}

        void set(int index, int row) {
            this->index = index;
            this->row = row;
        }
    };

    enum StackState {
        SearchNext,
        BackTracking,
        BackTrackingRetry,
        Last
    };

public:
    DancingLinks(size_t nodes)
        : list_(nodes), col_size_(this->cols() + 1), last_idx_(0) {
    }

    ~DancingLinks() {}

    bool is_empty() const { return (list_.next[0] == 0); }

    int cols() const { return (int)SudokuHelper::TotalConditions; }

    const std::vector<int> &              get_answer() const  { return this->answer_; }
    const std::vector<std::vector<int>> & get_answers() const { return this->answers_; }

    static size_t get_recur_counter() { return DancingLinks::recur_counter; }

private:
    int get_min_column() const {
        int first = list_.next[0];
        int min_col = col_size_[first];
        assert(min_col >= 0);
        if (min_col <= 1)
            return first;
        int min_col_index = first;
        for (int i = list_.next[first]; i != 0 ; i = list_.next[i]) {
            int col_size = col_size_[i];
            if (col_size < min_col) {
                assert(col_size >= 0);
#if 1
                if (col_size <= 0)
                    return 0;
#endif
                if (col_size <= 1)
                    return i;
                min_col = col_size;
                min_col_index = i;
            }
        }
        return min_col_index;
    }

    int get_max_column() const {
        int first = list_.next[0];
        int max_col = col_size_[first];
        assert(max_col >= 0);
        int max_col_index = first;
        for (int i = list_.next[first]; i != 0 ; i = list_.next[i]) {
            if (col_size_[i] > max_col) {
                assert(col_size_[i] >= 0);
                max_col = col_size_[i];
                max_col_index = i;
            }
        }
        return max_col_index;
    }

public:
    void init() {
        int cols = this->cols();
        for (int col = 0; col <= cols; col++) {
            list_.prev[col] = col - 1;
            list_.next[col] = col + 1;
            list_.up[col] = col;
            list_.down[col] = col;
            list_.row[col] = 0;
            list_.col[col] = col;
        }
        list_.prev[0] = cols;
        list_.next[cols] = 0;

        last_idx_ = cols + 1;
        for (size_t i = 0; i < col_size_.size(); i++) {
            col_size_[i] = 0;
        }
        //col_size_.resize(this->cols() + 1);

        this->answers_.clear();
        this->answer_.reserve(81);
#if V1B_SEARCH_ALL_STAGE
        this->answers_.clear();
#endif
        recur_counter = 0;
    }

    void build(const std::vector<std::vector<char>> & board) {
        size_t empties = 0;
        for (size_t row = 0; row < board.size(); row++) {
            const std::vector<char> & line = board[row];
            for (size_t col = 0; col < line.size(); col++) {
                if (line[col] == '.') {
                    empties++;
                }
            }
        }

        // maxRows = filled * 1 + empties * 9;
        //         = (9 * 9 - empties) * 1 + empties * 9;
        //         = (9 * 9) + empties * 8;
        size_t filled = Rows * Cols - empties;
        size_t maxRows = filled * 1 +  empties * Numbers;        

        int row_idx = 1;

        assert(Rows == board.size());
        for (size_t row = 0; row < board.size(); row++) {
            const std::vector<char> & line = board[row];
            assert(Cols == line.size());
            for (size_t col = 0; col < line.size(); col++) {
                size_t minNum, maxNum;
                if (line[col] == '.') {
                    minNum = 0, maxNum = Numbers - 1;
                }
                else {
                    minNum = maxNum = line[col] - '1';
                }
                size_t palace = row / 3 * 3 + col / 3;
                for (size_t number = minNum; number <= maxNum; number++) {
                    int head = last_idx_;
                    int index = last_idx_;

                    this->insert(index + 0, row_idx, (int)(0      + row * 9 + col + 1));
                    this->insert(index + 1, row_idx, (int)(81 * 1 + row * 9 + number + 1));
                    this->insert(index + 2, row_idx, (int)(81 * 2 + col * 9 + number + 1));
                    this->insert(index + 3, row_idx, (int)(81 * 3 + palace * 9 + number + 1));

                    this->rows_[row_idx] = row;
                    this->cols_[row_idx] = col;
                    this->numbers_[row_idx] = number;
                    index += 4;
                    row_idx++;

                    list_.next[index - 1] = head;
                    list_.prev[head] = index - 1;
                    last_idx_ = index;
                }
            }
        }
        assert(row_idx == (maxRows + 1));
    }

    void insert(int index, int row, int col) {
        list_.prev[index] = index - 1;
        list_.next[index] = index + 1;
        list_.up[index] = list_.up[col];
        list_.down[index] = col;
        list_.row[index] = row;
        list_.col[index] = col;
#if 1
        list_.down[list_.up[index]] = index;
        list_.up[col] = index;
#else
        list_.down[list_.up[index]] = index;
        list_.up[list_.down[index]] = index;
#endif
        col_size_[col]++;
    }

    void remove(int index) {
        assert(index > 0);
        int prev = list_.prev[index];
        int next = list_.next[index];
        list_.next[prev] = next;
        list_.prev[next] = prev;

        for (int row = list_.down[index]; row != index; row = list_.down[row]) {
            for (int col = list_.next[row]; col != row; col = list_.next[col]) {
                int up = list_.up[col];
                int down = list_.down[col];
                list_.down[up] = down;
                list_.up[down] = up;
                assert(col_size_[list_.col[col]] > 0);
                col_size_[list_.col[col]]--;
            }
        }
    }

    void restore(int index) {
        assert(index > 0);
        int next = list_.next[index];
        int prev = list_.prev[index];
        list_.prev[next] = index;
        list_.next[prev] = index;

        for (int row = list_.up[index]; row != index; row = list_.up[row]) {
            for (int col = list_.prev[row]; col != row; col = list_.prev[col]) {
                int down = list_.down[col];
                int up = list_.up[col];
                list_.up[down] = col;
                list_.down[up] = col;
                col_size_[list_.col[col]]++;
            }
        }
    }

    bool solve() {
        if (this->is_empty()) {
            if (kSearchAllStages)
                this->answers_.push_back(this->answer_);
            else
                return true;
        }

        recur_counter++;
        
        int index = get_min_column();
        assert(index > 0);

        if (index > 0) {
            this->remove(index);
            for (int row = list_.down[index]; row != index; row = list_.down[row]) {
                this->answer_.push_back(list_.row[row]);
                for (int col = list_.next[row]; col != row; col = list_.next[col]) {
                    this->remove(list_.col[col]);
                }

                if (this->solve()) {
                    if (!kSearchAllStages)
                        return true;
                }

                for (int col = list_.prev[row]; col != row; col = list_.prev[col]) {
                    this->restore(list_.col[col]);
                }
                this->answer_.pop_back();
            }
            this->restore(index);
        }

        return false;
    }

    bool solve_non_recursive() {
        int state = StackState::SearchNext;
        std::vector<StackInfo> stack;
        StackInfo stack_info;
        int index, row;

        do {       
            if (state == StackState::SearchNext) {
Search_Next:
                if (this->is_empty()) {
                    if (kSearchAllStages) {
                        this->answers_.push_back(answer_);
                        state = StackState::BackTracking;
                        goto BackTracking_Entry;
                    }
                    else {
                        return true;
                    }
                }

                recur_counter++;

                index = get_min_column();
                assert(index > 0);
                this->remove(index);

                row = list_.down[index];

                while (row != index) {
                    stack_info.set(index, row);
                    stack.push_back(stack_info);
                    this->answer_.push_back(list_.row[row]);

                    for (int col = list_.next[row]; col != row; col = list_.next[col]) {
                        this->remove(list_.col[col]);
                    }

                    // SearchNext
                    goto Search_Next;
                }
            }
            else if (state == StackState::BackTrackingRetry) {
BackTracking_Retry:
                while (row != index) {
                    stack_info.set(index, row);
                    stack.push_back(stack_info);
                    this->answer_.push_back(list_.row[row]);
                    for (int col = list_.next[row]; col != row; col = list_.next[col]) {
                        this->remove(list_.col[col]);
                    }

                    state = StackState::SearchNext;
                    goto Search_Next;
                }
            }
            else {
                // StackState::BackTracking
BackTracking_Entry:
                this->answer_.pop_back();
                stack_info = stack.back();
                stack.pop_back();
                index = stack_info.index;
                row = stack_info.row;

                for (int col = list_.prev[row]; col != row; col = list_.prev[col]) {
                    this->restore(list_.col[col]);
                }

                row = list_.down[row];

                state = StackState::BackTrackingRetry;
                goto BackTracking_Retry;
            }

            this->restore(index);

            if (stack.size() != 0) {
                state = StackState::BackTracking;
                goto BackTracking_Entry;
            }
            else break;
        } while (1);

        return false;
    }

    void display_answer(std::vector<std::vector<char>> & board) {
        for (auto idx : this->get_answer()) {
            if (idx > 0) {
                board[this->rows_[idx]][this->cols_[idx]] = (char)this->numbers_[idx] + '1';
            }
        }

        SudokuHelper::display_board(board);
    }

    void display_answers(std::vector<std::vector<char>> & board) {
        printf("Total answers: %d\n\n", (int)this->get_answers().size());
        int i = 0;
        for (auto answer : this->get_answers()) {
            SudokuHelper::clear_board(board);
            for (auto idx : answer) {
                board[this->rows_[idx]][this->cols_[idx]] = (char)this->numbers_[idx] + '1';
            }
            SudokuHelper::display_board(board, false, i);
            i++;
        }
    }
};

size_t DancingLinks::recur_counter = 0;

class Solution {
private:
    DancingLinks solver_;

public:
    Solution() : solver_(SudokuHelper::TotalSize * 4 + 1) {
    }
    ~Solution() {}

public:
    double solveSudoku(std::vector<std::vector<char>> & board, bool verbose = true) {
        double elapsed_time;
        if (verbose) {
            SudokuHelper::display_board(board, true);
        }

        jtest::StopWatch sw;
        sw.start();

        solver_.init();
        solver_.build(board);
        solver_.solve();
        //solver.solve_non_recursive();

        sw.stop();
        elapsed_time = sw.getElapsedMillisec();

        if (verbose) {
            if (kSearchAllStages)
                solver_.display_answers(board);
            else
                solver_.display_answer(board);
            printf("Elapsed time: %0.3f ms, recur_counter: %u\n\n",
                   elapsed_time, (uint32_t)DancingLinks::get_recur_counter());
        }

        return elapsed_time;
    }
};

} // namespace v1b
} // namespace Problem_37
} // namespace LeetCode

#endif // LEETCODE_SUDOKU_SOLVER_V1B_H
