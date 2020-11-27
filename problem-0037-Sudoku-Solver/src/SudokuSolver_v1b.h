
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#include "SudokuSolver.h"
#include "StopWatch.h"

#define V1B_SEARCH_ALL_ANSWERS  0

namespace LeetCode {
namespace Problem_37 {
namespace v1b {

#if V1B_SEARCH_ALL_ANSWERS
static const bool kSearchAllAnswers = true;
#else
static const bool kSearchAllAnswers = false;
#endif

//
// From: https://www.ituring.com.cn/article/265203
//

static const int XSIZE = 3;
static const int SIZE = XSIZE * XSIZE;
static const int MAX_C = SIZE * SIZE * 4;                    // 最大列 
static const int MAX_R = SIZE * SIZE * SIZE;                 // 最大行
static const int MAX_SUDOKU = SIZE * SIZE;                   // 数独矩阵大小
static const int MAX_LINK = (MAX_C + 1) * (MAX_R + 1);       // 链表最大范围

int L[MAX_LINK], R[MAX_LINK], U[MAX_LINK], D[MAX_LINK];      // 抽象链表
int Col[MAX_LINK], Row[MAX_LINK], col_size[MAX_C + 1], H[MAX_R + 1]; // C&O代表列&行，S每一列的节点数，H每一行的第一个节点
int last_idx, max_depth;                                     // 用来指向节点
size_t recur_counter;
int board[MAX_SUDOKU], answer[MAX_SUDOKU], record[MAX_SUDOKU];

void init(void);        // Dancing Links的抽象链表初始化
void add_nodes(int, int, int);
void insert(int, int);  // 在链表的一个位置中添加标记 
void remove(int);       // 删除一列，同时删除这一列中的行 
void restore(int);      // 恢复一列，同时恢复这一列中的行

bool input(void)
{
    char buffer[SIZE + 1][SIZE + 1]; // 留一个空间 
    if (scanf("%s", buffer[0]) == EOF)
        return false;
    for (int i = 1; i < SIZE; i++)
        scanf("%s", buffer[i]);
    memset(board, 0, sizeof(board));
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            if (buffer[row][col] != '0') {
                board[row * SIZE + col] = buffer[row][col] - '0';
            }
        }
    }
    return true;
}

char puzzle[SIZE * SIZE + 3]; // 留一个空间给换行符 

bool input_from_file(FILE * f)
{
    if (fgets(puzzle, SIZE * SIZE + 2, f) == NULL)
        return false;

    memset(board, 0, sizeof(board));
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            char val = puzzle[row * SIZE + col];
            if (val >= '0' && val <= '9')
                board[row * SIZE + col] = val - '0';
            /*
            else if (val == '.' || val == ' ' || val == '-')
                board[row * SIZE + col] = 0;
            //*/
        }
    }
    return true;
}

void read_sudoku_board(std::vector<std::vector<char>> & in_board)
{
    memset(board, 0, sizeof(board));
    for (int row = 0; row < SIZE; row++) {
        const std::vector<char> & line = in_board[row];
        for (int col = 0; col < SIZE; col++) {
            char val = line[col];
            if (val >= '0' && val <= '9')
                board[row * SIZE + col] = val - '0';
            else if (val == '.' || val == ' ' || val == '-')
                board[row * SIZE + col] = 0;
        }
    }
}

void init(void)
{
    for (int col = 0; col <= MAX_C; col++) {
        L[col] = col - 1;
        R[col] = col + 1;
        U[col] = col;
        D[col] = col;
        Col[col] = col;
        Row[col] = 0;
    }
    L[0] = MAX_C;
    R[MAX_C] = 0;
    last_idx = MAX_C + 1;

    recur_counter = 0;
    max_depth = 0;
    memset(col_size, 0, sizeof(col_size));
    memset(H, 0, sizeof(H));
    memset(record, 0, sizeof(record));
}

void build(void)
{
    int pos;
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            pos = row * SIZE + col;
            if (board[pos] != 0) {
                add_nodes(row, col, board[pos] - 1);
            }
            else if (board[pos] == 0) {
                for (int num = 0; num < SIZE; num++) {
                    add_nodes(row, col, num);
                }
            }
        }
    }
}

void add_nodes(int row, int col, int num)
{
    int index = row * MAX_SUDOKU + col * SIZE + num + 1;
    insert(index, MAX_SUDOKU * 0 + row * SIZE + col + 1);
    insert(index, MAX_SUDOKU * 1 + row * SIZE + num + 1);
    insert(index, MAX_SUDOKU * 2 + col * SIZE + num + 1);
    insert(index, MAX_SUDOKU * 3 + (row / XSIZE * XSIZE + col / XSIZE) * SIZE + num + 1);
}

void insert(int row, int col)
{
    if (H[row]) {
        L[last_idx] = L[H[row]];
        R[last_idx] = H[row];
        L[R[last_idx]] = last_idx;
        R[L[last_idx]] = last_idx;
    }
    else {
        L[last_idx] = last_idx;
        R[last_idx] = last_idx;
        H[row] = last_idx;
    }

    U[last_idx] = U[col];
    D[last_idx] = col;
    U[D[last_idx]] = last_idx;
    D[U[last_idx]] = last_idx;
    Col[last_idx] = col;
    Row[last_idx] = row;

    col_size[col]++;
    last_idx++;
}

void remove(int index)
{
    L[R[index]] = L[index];
    R[L[index]] = R[index];
    for (int row = D[index]; row != index; row = D[row]) {
        for (int col = R[row]; col != row; col = R[col]) {
            U[D[col]] = U[col];
            D[U[col]] = D[col];
            col_size[Col[col]]--;
        }
    }
}

void restore(int index)
{
    for (int row = U[index]; row != index; row = U[row]) {
        for (int col = L[row]; col != row; col = L[col]) {
            U[D[col]] = col;
            D[U[col]] = col;
            col_size[Col[col]]++;
        }
    }
    L[R[index]] = index;
    R[L[index]] = index;
}

bool dfs(int depth)
{
    if (R[0] == 0) {
        max_depth = depth;
        return true;
    }

    recur_counter++;

    int count = ~(1 << 31), index = 0;
    for (int i = R[0]; i != 0; i = R[i]) {
        int csize = col_size[i];
        if (csize < count) {
#if 0
            if (csize == 0)
                return false;
#endif
            count = csize;
            index = i;
            if (count == 1)
                break;
        }
    }

    assert(index > 0);
    //if (index > 0) {
        remove(index);
        for (int row = D[index]; row != index; row = D[row]) {
            for (int col = R[row]; col != row; col = R[col]) {
                remove(Col[col]);
            }
            record[depth] = Row[row];

            if (dfs(depth + 1))
                return true;

            for (int col = L[row]; col != row; col = L[col]) {
                restore(Col[col]);
            }
        }
        restore(index);
    //}
    return false;
}

void output(void)
{
    for (int i = 0; i < max_depth; i++) {
        answer[(record[i] - 1) / SIZE] = (record[i] - 1) % SIZE + 1;
    }
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++)
            printf("%-2d", answer[row * SIZE + col]);
        printf("\n");
    }
}

void output_board(FILE * f)
{
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++)
            fprintf(f, "%-2d", board[row * SIZE + col]);
        fprintf(f, "\n");
    }
    fprintf(f, "\n");
}

void output_to_file(FILE * f)
{
    for (int i = 0; i < max_depth; i++) {
        answer[(record[i] - 1) / SIZE] = (record[i] - 1) % SIZE + 1;
    }
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++)
            fprintf(f, "%-2d", answer[row * SIZE + col]);
        fprintf(f, "\n");
    }
    fprintf(f, "\n");
}

void display_answer(std::vector<std::vector<char>> & in_board) {
    for (int i = 0; i < max_depth; i++) {
        answer[(record[i] - 1) / SIZE] = (record[i] - 1) % SIZE + 1;
    }

    in_board.clear();
    for (int row = 0; row < SIZE; row++) {
        std::vector<char> line;
        for (int col = 0; col < SIZE; col++) {
            int num = answer[row * SIZE + col];
            if (num != 0)
                line.push_back((char)(num + '0'));
            else
                line.push_back('.');
        }
        in_board.emplace_back(line);
    }

    SudokuHelper::display_board(in_board);
}

bool solve_sudoku(std::vector<std::vector<char>> & in_board,
                  double & elapsed_time, bool verbose)
{
    if (verbose) {
        SudokuHelper::display_board(in_board, true);
    }

    read_sudoku_board(in_board);

    jtest::StopWatch sw;
    sw.start();

    init();
    build();
    bool success = dfs(0);
        
    sw.stop();
    elapsed_time = sw.getElapsedMillisec();

    if (verbose) {
        display_answer(in_board);
        printf("Elapsed time: %0.3f ms, recur_counter: %u\n\n",
               elapsed_time, (uint32_t)recur_counter);
    }
    return success;
}

int run_test_sudoku17(const char * filename, const char * output_file)
{
    size_t puzzleCount = 0;
    FILE * file = fopen(filename, "rt");
    FILE * out_file = fopen(output_file, "wt");

    jtest::StopWatch sw_total;
    sw_total.start();
    while (input_from_file(file)) {
        fprintf(out_file, "%s\n", puzzle);
        //output_board(out_file);

        jtest::StopWatch sw;
        sw.start();

        init();
        build();
        bool success = dfs(0);
        
        sw.stop();
        double elapsed_time = sw.getElapsedMillisec();

        output_to_file(out_file);
        fprintf(out_file, "Time: %0.3f ms\n\n", elapsed_time);
        //fflush(out_file);

        if (success) {
            puzzleCount++;
#ifndef NDEBUG
            if (puzzleCount > 10000)
                break;
#endif
        }
    }
    sw_total.stop();
    
    fprintf(out_file, "puzzleCount: %" PRIuPTR "\n\n", puzzleCount);
    fprintf(out_file, "Total Time: %0.3f ms\n\n", sw_total.getElapsedMillisec());

    printf("puzzleCount: %" PRIuPTR "\n\n", puzzleCount);
    printf("Total Time: %0.3f ms\n\n", sw_total.getElapsedMillisec());

    fclose(file);
    fclose(out_file);
    return 0;
}

} // namespace v1b
} // namespace Problem_37
} // namespace LeetCode

#endif // LEETCODE_SUDOKU_SOLVER_V1B_H
