
#ifndef LEETCODE_SUDOKU_SOLVER_V1A_H
#define LEETCODE_SUDOKU_SOLVER_V1A_H

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

#define V1A_SEARCH_ALL_STAGE    0
#define V1A_INSERT_MODE         1

namespace LeetCode {
namespace Problem_37 {
namespace v1a {

#if V1A_SEARCH_ALL_STAGE
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
    size_t empties_;
    size_t rows[TotalSize + 1];
    size_t cols[TotalSize + 1];
    size_t numbers[TotalSize + 1];

public:
    SudokuSolver(const std::vector<std::vector<char>> & board) : empties_(0) {
        this->init(board);
    }
    ~SudokuSolver() {}

    matrix_type & getDlkMatrix() {
        return this->matrix;
    }

    const matrix_type & getDlkMatrix() const {
        return this->matrix;
    }

    size_t getEmpties() const {
        return this->empties_;
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

        this->empties_ = empties;

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
    template <typename TDancingLinksX>
    void display_answer(std::vector<std::vector<char>> & board,
                        const TDancingLinksX * dancingLinks);

    template <typename TDancingLinksX>
    void display_answers(std::vector<std::vector<char>> & board,
                         const TDancingLinksX * dancingLinks);
};

class DancingLinks {
private:
#if 0
    struct Node {
        int prev, next;
        int up, down;
        int row, col;
    };
#else
    struct alignas(32) Node {
        int prev, next;
        int up, down;
        int row, col;
        int reserve1, reserve2;
    };
#endif
    
    std::vector<Node>   list_;
    std::vector<int>    col_size;
    std::vector<int>    answer;
    int                 last_idx;

    std::vector<std::vector<int>> answers;

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

        this->answer.reserve(81);
        recur_counter = 0;
    }

#if (V1A_INSERT_MODE == 0)

    void init_from_matrix(const typename SudokuSolver::matrix_type & matrix) {
        int rows = (int)matrix.rows();
        int cols = (int)matrix.cols();
        for (int row = rows - 1; row >= 0; row--) {
            int first = last_idx;
            int index = last_idx;
            for (int col = 0; col < cols; col++) {
                if (matrix[row].test(col)) {
                    this->insert(index, row + 1, col + 1);
                    index++;
                }
            }
            list_[index - 1].next = first;
            list_[first].prev = index - 1;
            last_idx = index;
        }
    }

    void insert(int index, int row, int col) {
        list_[index].prev = index - 1;
        list_[index].next = index + 1;
        list_[index].up = col;
        list_[index].down = list_[col].down;
        list_[index].row = row;
        list_[index].col = col;
#if 1
        list_[col].down = index;
        list_[list_[index].down].up = index;
#else
        list_[list_[index].up].down = index;
        list_[list_[index].down].up = index;
#endif
        col_size[col]++;
    }

#else // !(V1A_INSERT_MODE == 0)

    void init_from_matrix(const typename SudokuSolver::matrix_type & matrix) {
        int rows = (int)matrix.rows();
        int cols = (int)matrix.cols();
        for (int row = 0; row < rows; row++) {
            int first = last_idx;
            int index = last_idx;
            for (int col = 0; col < cols; col++) {
                if (matrix[row].test(col)) {
                    this->insert(index, row + 1, col + 1);
                    index++;
                }
            }
            list_[index - 1].next = first;
            list_[first].prev = index - 1;
            last_idx = index;
        }
    }

    void insert(int index, int row, int col) {
        list_[index].prev = index - 1;
        list_[index].next = index + 1;
        list_[index].up = list_[col].up;
        list_[index].down = col;
        list_[index].row = row;
        list_[index].col = col;
#if 1
        list_[list_[index].up].down = index;
        list_[col].up = index;
#else
        list_[list_[index].up].down = index;
        list_[list_[index].down].up = index;
#endif
        col_size[col]++;
    }

#endif // (V1A_INSERT_MODE == 0)

    int get_min_column() const {
        int first = list_[0].next;
        int min_col = col_size[first];
        assert(min_col >= 0);
        if (min_col <= 1)
            return first;
        int min_col_index = first;
        for (int i = list_[first].next; i != 0 ; i = list_[i].next) {
            if (col_size[i] < min_col) {
                assert(col_size[i] >= 0);
                if (col_size[i] <= 1)
                    return i;
                min_col = col_size[i];
                min_col_index = i;
            }
        }
        return min_col_index;
    }

    int get_max_column() const {
        int first = list_[0].next;
        int max_col = col_size[first];
        assert(max_col >= 0);
        int max_col_index = first;
        for (int i = list_[first].next; i != 0 ; i = list_[i].next) {
            if (col_size[i] > max_col) {
                assert(col_size[i] >= 0);
                max_col = col_size[i];
                max_col_index = i;
            }
        }
        return max_col_index;
    }

public:
    void remove(int index) {
        assert(index > 0);
        int prev = list_[index].prev;
        int next = list_[index].next;
        list_[prev].next = next;
        list_[next].prev = prev;

        for (int row = list_[index].down; row != index; row = list_[row].down) {
            for (int col = list_[row].next; col != row; col = list_[col].next) {
                int up = list_[col].up;
                int down = list_[col].down;
                list_[up].down = down;
                list_[down].up = up;
                assert(col_size[list_[col].col] > 0);
                col_size[list_[col].col]--;
            }
        }
    }

    void restore(int index) {
        assert(index > 0);
        int next = list_[index].next;
        int prev = list_[index].prev;
        list_[next].prev = index;
        list_[prev].next = index;

        for (int row = list_[index].up; row != index; row = list_[row].up) {
            for (int col = list_[row].prev; col != row; col = list_[col].prev) {
                int down = list_[col].down;
                int up = list_[col].up;
                list_[down].up = col;
                list_[up].down = col;
                col_size[list_[col].col]++;
            }
        }
    }

    bool search() {
        if (this->is_empty()) {
            if (kSearchAllStages)
                this->answers.push_back(this->answer);
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
                if (!kSearchAllStages)
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
                    if (kSearchAllStages) {
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

class DLX {
private:
    struct Node {
        int prev, next;
        int up, down;
        int row, col;
    };

    std::vector<Node>   list_;
    std::vector<int>    col_size;
    std::vector<int>    answer_;
    int                 last_idx;

    std::vector<std::vector<int>> answers_;

public:
    DLX(typename SudokuSolver::matrix_type & matrix, size_t nodes)
        : list_(nodes), col_size(matrix.cols() + 1), last_idx((int)matrix.cols() + 1) {
        this->init(matrix);
    }
    ~DLX() {}

    bool is_empty() const { return (list_[0].next == 0); }

    const std::vector<int> &              get_answer() const  { return this->answer_; }
    const std::vector<std::vector<int>> & get_answers() const { return this->answers_; }

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

        this->answer_.resize(81);
    }

    void init_from_matrix(const typename SudokuSolver::matrix_type & matrix) {
        int rows = (int)matrix.rows();
        int cols = (int)matrix.cols();
        for (int row = 0; row < rows; row++) {
            int first = last_idx;
            int index = last_idx;
            for (int col = 0; col < cols; col++) {
                if (matrix[row].test(col)) {
                    this->insert(index, row + 1, col + 1);
                    index++;
                }
            }
            list_[index - 1].next = first;
            list_[first].prev = index - 1;
            last_idx = index;
        }
    }

    void insert(int index, int row, int col) {
        list_[index].prev = index - 1;
        list_[index].next = index + 1;
        list_[index].up = list_[col].up;
        list_[index].down = col;
        list_[index].row = row;
        list_[index].col = col;
#if 1
        list_[list_[index].up].down = index;
        list_[col].up = index;
#else
        list_[list_[index].up].down = index;
        list_[list_[index].down].up = index;
#endif
        col_size[col]++;
    }

    int get_min_column() const {
        int first = list_[0].next;
        int min_col = col_size[first];
        assert(min_col >= 0);
        if (min_col <= 1)
            return first;
        int min_col_index = first;
        for (int i = list_[first].next; i != 0 ; i = list_[i].next) {
            if (col_size[i] < min_col) {
                assert(col_size[i] >= 0);
                if (col_size[i] <= 1)
                    return i;
                min_col = col_size[i];
                min_col_index = i;
            }
        }
        return min_col_index;
    }

public:
    void remove(int index) {
        assert(index > 0);
        int prev = list_[index].prev;
        int next = list_[index].next;
        list_[prev].next = next;
        list_[next].prev = prev;

        for(int row = list_[index].down; row != index; row = list_[row].down) {
            for(int col = list_[row].next; col != row; col = list_[col].next) {
                int up = list_[col].up;
                int down = list_[col].down;
                list_[up].down = down;
                list_[down].up = up;
                assert(col_size[list_[col].col] > 0);
                col_size[list_[col].col]--;
            }
        }
    }

    void restore(int index) {
        assert(index > 0);
        int next = list_[index].next;
        int prev = list_[index].prev;
        list_[next].prev = index;
        list_[prev].next = index;

        for(int row = list_[index].up; row != index; row = list_[row].up) {
            for(int col = list_[row].prev; col != row; col = list_[col].prev) {
                int down = list_[col].down;
                int up = list_[col].up;
                list_[down].up = col;
                list_[up].down = col;
                col_size[list_[col].col]++;
            }
        }
    }

    bool search(size_t depth) {
        if (this->is_empty()) {
            if (kSearchAllStages)
                this->answers_.push_back(this->answer_);
            else
                return true;
        }

        int index = get_min_column();
        assert(index > 0);

        this->remove(index);
        for (int row = list_[index].down; row != index; row = list_[row].down) {
            this->answer_[depth] = list_[row].row;
            for (int col = list_[row].next; col != row; col = list_[col].next) {
                this->remove(list_[col].col);
            }

            if (this->search(depth + 1)) {
                if (!kSearchAllStages)
                    return true;
            }

            for (int col = list_[row].prev; col != row; col = list_[col].prev) {
                this->restore(list_[col].col);
            }
            //this->answer_[depth] = 0;
        }
        this->restore(index);

        return false;
    }

    bool solve() {
        if (!this->search(0)) {
            return false;
        }
        return true;
    }
};

class Solution {
public:
    double solveSudoku(std::vector<std::vector<char>> & board, bool verbose = true);
};

} // namespace v1a
} // namespace Problem_37
} // namespace LeetCode

#endif // LEETCODE_SUDOKU_SOLVER_V1A_H
