
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

#define V1_SEARCH_ALL_STAGE     0

namespace LeetCode {
namespace Problem_37 {
namespace v1 {

#if V1_SEARCH_ALL_STAGE
static const bool kSearchAllStages = true;
#else
static const bool kSearchAllStages = false;
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
    
    std::vector<Node>   linked_list;
    std::vector<int>    col_size;
    int                 last_idx;
    std::vector<int>    answer;

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
        : linked_list(nodes), col_size(matrix.cols() + 1), last_idx((int)matrix.cols() + 1) {
        this->init(matrix);
    }

    ~DancingLinks() {}

    bool is_empty() const { return (linked_list[0].next == 0); }

    const std::vector<int> &              get_answer() const  { return this->answer; }
    const std::vector<std::vector<int>> & get_answers() const { return this->answers; }

private:
    void init(const typename SudokuSolver::matrix_type & matrix) {
        int cols = (int)matrix.cols();
        for (int col = 0; col <= cols; col++) {
            linked_list[col].prev = col - 1;
            linked_list[col].next = col + 1;
            linked_list[col].up = col;
            linked_list[col].down = col;
            linked_list[col].row = 0;
            linked_list[col].col = col;
        }
        linked_list[0].prev = cols;
        linked_list[cols].next = 0;

        init_from_matrix(matrix);
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
        linked_list[last_idx].prev = tail;
        linked_list[last_idx].next = head;
        linked_list[last_idx].up = col;
        linked_list[last_idx].down = linked_list[col].down;
        linked_list[last_idx].row = row;
        linked_list[last_idx].col = col;
#if 1
        linked_list[tail].next = last_idx;
        linked_list[head].prev = last_idx;
        linked_list[col].down = last_idx;
        linked_list[linked_list[last_idx].down].up = last_idx;
#else
        linked_list[linked_list[last_idx].prev].next = last_idx;
        linked_list[linked_list[last_idx].next].prev = last_idx;
        linked_list[linked_list[last_idx].up].down = last_idx;
        linked_list[linked_list[last_idx].down].up = last_idx;
#endif
        col_size[col]++;
        tail = last_idx++;
        return tail;
    }

    int get_min_column() const {
        int first = linked_list[0].next;
        int min_col = col_size[first];
        int min_col_index = first;
        for (int i = linked_list[0].next; i != 0 ; i = linked_list[i].next) {
            if (col_size[i] < min_col) {
                min_col = col_size[i];
                min_col_index = i;
            }
        }
        return min_col_index;
    }

public:
    void remove(int index) {
        linked_list[linked_list[index].prev].next = linked_list[index].next;
        linked_list[linked_list[index].next].prev = linked_list[index].prev;

        for (int row = linked_list[index].down; row != index; row = linked_list[row].down) {
            for (int col = linked_list[row].next; col != row; col = linked_list[col].next) {
                linked_list[linked_list[col].up].down = linked_list[col].down;
                linked_list[linked_list[col].down].up = linked_list[col].up;
                col_size[linked_list[col].col]--;
                assert(col_size[linked_list[col].col] >= 0);
                if (col_size[linked_list[col].col] == 0)
                    col = col;
            }
        }
    }

    void restore(int index) {
        for (int row = linked_list[index].up; row != index; row = linked_list[row].up) {
            for (int col = linked_list[row].prev; col != row; col = linked_list[col].prev) {
                linked_list[linked_list[col].down].up = col;
                linked_list[linked_list[col].up].down = col;
                col_size[linked_list[col].col]++;
            }
        }

        linked_list[linked_list[index].next].prev = index;
        linked_list[linked_list[index].prev].next = index;
    }

    bool solve() {
        if (this->is_empty()) {
            if (kSearchAllStages)
                this->answers.push_back(answer);
            else
                return true;
        }
        
        int index = get_min_column();
        assert(index > 0);
        this->remove(index);
        for (int row = linked_list[index].down; row != index; row = linked_list[row].down) {
            this->answer.push_back(linked_list[row].row);
            for (int col = linked_list[row].next; col != row; col = linked_list[col].next) {
                this->remove(linked_list[col].col);
            }

            if (solve()) {
                if (!kSearchAllStages)
                    return true;
            }

            for (int col = linked_list[row].prev; col != row; col = linked_list[col].prev) {
                this->restore(linked_list[col].col);
            }
            this->answer.pop_back();
        }
        this->restore(index);
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
                        this->answers.push_back(answer);
                        state = StackState::BackTracking;
                        goto BackTracking_Entry;
                    }
                    else {
                        return true;
                    }
                }

                index = get_min_column();
                assert(index > 0);
                this->remove(index);

                row = linked_list[index].down;

                while (row != index) {
                    stack_info.set(index, row);
                    stack.push_back(stack_info);
                    this->answer.push_back(linked_list[row].row);

                    for (int col = linked_list[row].next; col != row; col = linked_list[col].next) {
                        this->remove(linked_list[col].col);
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
                    this->answer.push_back(linked_list[row].row);
                    for (int col = linked_list[row].next; col != row; col = linked_list[col].next) {
                        this->remove(linked_list[col].col);
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

                for (int col = linked_list[row].prev; col != row; col = linked_list[col].prev) {
                    this->restore(linked_list[col].col);
                }

                row = linked_list[row].down;

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
};

class Solution {
public:
    void solveSudoku(std::vector<std::vector<char>> & board);
};

} // namespace v1
} // namespace Problem_37
} // namespace LeetCode

#endif // LEETCODE_SUDOKU_SOLVER_V1_H
