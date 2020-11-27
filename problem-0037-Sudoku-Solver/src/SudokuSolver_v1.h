
#ifndef LEETCODE_SUDOKU_SOLVER_V1_H
#define LEETCODE_SUDOKU_SOLVER_V1_H

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

#define V1_SEARCH_ALL_ANSWERS   0

namespace LeetCode {
namespace Problem_37 {
namespace v1 {

#if V1_SEARCH_ALL_ANSWERS
static const bool kSearchAllAnswers = true;
#else
static const bool kSearchAllAnswers = false;
#endif

class DancingLinks;

class SudokuSolver {
public:
    static const size_t Rows = SudokuHelper::Rows;
    static const size_t Cols = SudokuHelper::Cols;
    static const size_t Palaces = SudokuHelper::Palaces;
    static const size_t Numbers = SudokuHelper::Numbers;

    static const size_t TotalSize = Rows * Cols * Numbers;

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

    typedef SmallBitMatrix<TotalSize, TotalConditions> matrix_type;

private:
    matrix_type matrix;
    size_t rows[TotalSize + 1];
    size_t cols[TotalSize + 1];
    size_t numbers[TotalSize + 1];

public:
    SudokuSolver(const std::vector<std::vector<char>> & board) {
        this->init(board);
    }
    ~SudokuSolver() {}

    matrix_type & getDlkMatrix() {
        return this->matrix;
    }

    const matrix_type & getDlkMatrix() const {
        return this->matrix;
    }

private:
    void init(const std::vector<std::vector<char>> & board) {
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
        this->matrix.setRows(maxRows);

        size_t index = 0;
        this->matrix.clear();
        assert(Rows == board.size());
        for (size_t row = 0; row < board.size(); row++) {
            const std::vector<char> & line = board[row];
            assert(Cols == line.size());
            for (size_t col = 0; col < line.size(); col++) {
                size_t minNum = 0, maxNum = Numbers - 1;
                if (line[col] != '.') {
                    minNum = maxNum = line[col] - '1';
                }
                size_t palace = row / 3 * 3 + col / 3;
                for (size_t number = minNum; number <= maxNum; number++) {
#if (MATRIX_BITSET_MODE == MATRIX_USE_SMALL_BITMAP) || (MATRIX_BITSET_MODE == MATRIX_USE_BITMAP)
                    this->matrix[index].set(81 * 0 + row * 9 + col);
                    this->matrix[index].set(81 * 1 + row * 9 + number);
                    this->matrix[index].set(81 * 2 + col * 9 + number);
                    this->matrix[index].set(81 * 3 + (row / 3 * 3 + col / 3) * 9 + number);
#else
                    this->matrix[index][0      + row * 9 + col] = true;
                    this->matrix[index][81 * 1 + row * 9 + number] = true;
                    this->matrix[index][81 * 2 + col * 9 + number] = true;
                    this->matrix[index][81 * 3 + palace * 9 + number] = true;
#endif
                    this->rows[index + 1] = row;
                    this->cols[index + 1] = col;
                    this->numbers[index + 1] = number;
                    index++;
                }
            }
        }
        assert(index == maxRows);
    }

public:
    void display_answer(std::vector<std::vector<char>> & board,
                        const DancingLinks * dancingLinks);

    void display_answers(std::vector<std::vector<char>> & board,
                         const DancingLinks * dancingLinks);
};

class DancingLinks {
private:
    struct Node {
        int prev, next;
        int up, down;
        int row, col;
    };
    
    std::vector<Node>   list_;
    std::vector<int>    col_size;
    int                 last_idx;
    std::vector<int>    answer;

    static size_t recur_counter;

    std::vector<std::vector<int>> answers;

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
    DancingLinks(typename SudokuSolver::matrix_type & matrix, size_t nodes)
        : list_(nodes), col_size(matrix.cols() + 1), last_idx((int)matrix.cols() + 1) {
        this->init(matrix);
    }

    ~DancingLinks() {}

    bool is_empty() const { return (list_[0].next == 0); }

    const std::vector<int> &              get_answer() const  { return this->answer; }
    const std::vector<std::vector<int>> & get_answers() const { return this->answers; }

private:
    void init(const typename SudokuSolver::matrix_type & matrix) {
        int cols = (int)matrix.cols();
        for (int col = 0; col <= cols; col++) {
            list_[col].prev = col - 1;
            list_[col].next = col + 1;
            list_[col].up = col;
            list_[col].down = col;
            list_[col].row = 0;
            list_[col].col = col;
        }
        list_[0].prev = cols;
        list_[cols].next = 0;

        init_from_matrix(matrix);

        recur_counter = 0;
    }

    void init_from_matrix(const typename SudokuSolver::matrix_type & matrix) {
        int rows = (int)matrix.rows();
        int cols = (int)matrix.cols();
        for (int row = rows - 1; row >= 0; row--) {
        //for (int row = 0; row < rows; row++) {
            int head = last_idx, tail = last_idx;
            for (int col = 0; col < cols; col++) {
                if (matrix[row].test(col)) {
                    tail = this->insert(head, tail, row + 1, col + 1);
                }
            }
        }
    }

    int insert(int head, int tail, int row, int col) {
        list_[last_idx].prev = tail;
        list_[last_idx].next = head;
        list_[last_idx].up = col;
        list_[last_idx].down = list_[col].down;
        list_[last_idx].row = row;
        list_[last_idx].col = col;
#if 1
        list_[tail].next = last_idx;
        list_[head].prev = last_idx;
        list_[col].down = last_idx;
        list_[list_[last_idx].down].up = last_idx;
#else
        list_[list_[last_idx].prev].next = last_idx;
        list_[list_[last_idx].next].prev = last_idx;
        list_[list_[last_idx].up].down = last_idx;
        list_[list_[last_idx].down].up = last_idx;
#endif
        col_size[col]++;
        tail = last_idx++;
        return tail;
    }

    int get_min_column() const {
        int first = list_[0].next;
        int min_col = col_size[first];
        int min_col_index = first;
        for (int i = list_[0].next; i != 0 ; i = list_[i].next) {
            if (col_size[i] < min_col) {
                min_col = col_size[i];
                min_col_index = i;
            }
        }
        return min_col_index;
    }

public:
    void remove(int index) {
        list_[list_[index].prev].next = list_[index].next;
        list_[list_[index].next].prev = list_[index].prev;

        for (int row = list_[index].down; row != index; row = list_[row].down) {
            for (int col = list_[row].next; col != row; col = list_[col].next) {
                list_[list_[col].up].down = list_[col].down;
                list_[list_[col].down].up = list_[col].up;
                col_size[list_[col].col]--;
                assert(col_size[list_[col].col] >= 0);
                if (col_size[list_[col].col] == 0)
                    col = col;
            }
        }
    }

    void restore(int index) {
        for (int row = list_[index].up; row != index; row = list_[row].up) {
            for (int col = list_[row].prev; col != row; col = list_[col].prev) {
                list_[list_[col].down].up = col;
                list_[list_[col].up].down = col;
                col_size[list_[col].col]++;
            }
        }

        list_[list_[index].next].prev = index;
        list_[list_[index].prev].next = index;
    }

    bool search() {
        if (this->is_empty()) {
            if (kSearchAllAnswers)
                this->answers.push_back(answer);
            else
                return true;
        }

        recur_counter++;
        
        int index = get_min_column();
        assert(index > 0);
        this->remove(index);
        for (int row = list_[index].down; row != index; row = list_[row].down) {
            this->answer.push_back(list_[row].row);
            for (int col = list_[row].next; col != row; col = list_[col].next) {
                this->remove(list_[col].col);
            }

            if (search()) {
                if (!kSearchAllAnswers)
                    return true;
            }

            for (int col = list_[row].prev; col != row; col = list_[col].prev) {
                this->restore(list_[col].col);
            }
            this->answer.pop_back();
        }
        this->restore(index);
        return false;
    }

    bool search_non_recursive() {
        int state = StackState::SearchNext;
        std::vector<StackInfo> stack;
        StackInfo stack_info;
        int index, row;

        do {       
            if (state == StackState::SearchNext) {
Search_Next:
                if (this->is_empty()) {
                    if (kSearchAllAnswers) {
                        this->answers.push_back(answer);
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

                row = list_[index].down;

                while (row != index) {
                    stack_info.set(index, row);
                    stack.push_back(stack_info);
                    this->answer.push_back(list_[row].row);

                    for (int col = list_[row].next; col != row; col = list_[col].next) {
                        this->remove(list_[col].col);
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
                    this->answer.push_back(list_[row].row);
                    for (int col = list_[row].next; col != row; col = list_[col].next) {
                        this->remove(list_[col].col);
                    }

                    state = StackState::SearchNext;
                    goto Search_Next;
                }
            }
            else {
                // StackState::BackTracking
BackTracking_Entry:
                this->answer.pop_back();
                stack_info = stack.back();
                stack.pop_back();
                index = stack_info.index;
                row = stack_info.row;

                for (int col = list_[row].prev; col != row; col = list_[col].prev) {
                    this->restore(list_[col].col);
                }

                row = list_[row].down;

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

    bool solve(size_t & counter) {
        bool success = this->search();
        counter = recur_counter;
        return success;
    }

    bool solve_non_recursive(size_t & counter) {
        bool success = this->search_non_recursive();
        counter = recur_counter;
        return success;
    }
};

class Solution {
public:
    bool solveSudoku(std::vector<std::vector<char>> & board,
                     double & elapsed_time,
                     bool verbose = true);
};

} // namespace v1
} // namespace Problem_37
} // namespace LeetCode

#endif // LEETCODE_SUDOKU_SOLVER_V1_H
