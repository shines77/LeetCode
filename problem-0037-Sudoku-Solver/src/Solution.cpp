
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <cstring>      // For std::memset()
#include <vector>
#include <bitset>

#include "SudokuSolver_v1.h"
#include "SudokuSolver_v1a.h"
#include "SudokuSolver_v1b.h"
#include "SudokuSolver_v1c.h"
#include "SudokuSolver_v1d.h"
#include "SudokuSolver_v2.h"
#include "SudokuSolver_v3.h"
#include "SudokuSolver_v4.h"
#include "SudokuSolver_v5.h"
#include "SudokuSolver_v6.h"
#include "SudokuSolver_v7.h"
#include "SudokuSolver_v8.h"

#include "ice_sudoku_solver.h"

#include "CPUWarmUp.h"
#include "StopWatch.h"

using namespace LeetCode::Problem_37;

static const bool kEnableV1Solution =   true;
static const bool kEnableV1aSolution =  true;
static const bool kEnableV1bSolution =  true;
static const bool kEnableV1cSolution =  true;
static const bool kEnableV1dSolution =  true;
static const bool kEnableV2Solution =   true;
static const bool kEnableV3Solution =   true;
static const bool kEnableV4Solution =   true;
static const bool kEnableV5Solution =   true;
static const bool kEnableV6Solution =   true;
static const bool kEnableV7Solution =   false;
static const bool kEnableV8Solution =   true;
static const bool kEnableIceSudoku  =   false;

// Index: [0 - 4]
#define TEST_CASE_INDEX         4

struct SudokuBoard {
    const char * rows[9];
};

static SudokuBoard test_case[] = {
    //
    // 0 #
    //
    // Normal (filled = 30)
    // https://leetcode-cn.com/problems/sudoku-solver/
    //
    {
        "53. | .7. | ...",
        "6.. | 195 | ...",
        ".98 | ... | .6.",

        "8.. | .6. | ..3",
        "4.. | 8.3 | ..1",
        "7.. | .2. | ..6",

        ".6. | ... | 28.",
        "... | 419 | ..5",
        "... | .8. | .79",
    },

    //
    // 1 #
    //
    // Advance (filled = 24)
    // https://www.sudoku-cn.com/
    //
    {
        "4.2 | ... | 9..",
        "... | .61 | ...",
        ".19 | ... | ...",

        "7.5 | ... | 6..",
        "2.4 | 7.. | ..5",
        "... | .9. | 7..",

        ".8. | 2.9 | .1.",
        "..7 | ..4 | ...",
        "... | ... | .52",
    },

    //
    // 2 #
    //
    // Hard (filled = 21)
    // https://github.com/tropicalwzc/ice_sudoku.github.io/blob/master/sudoku_solver.c
    //
    {
        "5.. | ... | 3..",
        ".2. | 1.. | .7.",
        "..8 | ... | ..9",

        ".4. | ..7 | ...",
        "... | 821 | ...",
        "... | 6.. | .1.",

        "3.. | ... | 8..",
        ".6. | ..4 | .2.",
        "..9 | ... | ..5",
    },

    // Ice sudoku string: 500000300020100070008000009040007000000821000000600010300000800060004020009000005

    //
    // 3 #
    //
    // Hardcore (filled = 17)
    // http://www.cn.sudokupuzzle.org/play.php
    //
    {
        "5.. | ... | ...",
        ".1. | ... | 32.",
        "... | 84. | ...",

        "... | ... | ...",
        "... | ..3 | 1..",
        "6.8 | 5.. | ...",

        "..7 | ... | .68",
        ".34 | ..1 | ...",
        "... | ... | ...",
    },

    // Ice sudoku string: 500000000010000320000840000000000000000003100608500000007000068034001000000000000

    //
    // 4 #
    //
    // Hardcore (filled = 21)
    // http://news.sohu.com/20130527/n377158555.shtml
    // https://baike.baidu.com/reference/13848819/1bf4HJzRCPCNz9Rypz3HpTtnhc2MpcRr5JMIp0032uiuKPQm4eOMuq2WZWxf77V-UBRjIkyDf9CVZDEjlDeHJBaazlstk30qaDtt
    //
    {
        "8.. | ... | ...",
        "..3 | 6.. | ...",
        ".7. | .9. | 2..",

        ".5. | ..7 | ...",
        "... | .45 | 7..",
        "... | 1.. | .3.",

        "..1 | ... | .68",
        "..8 | 5.. | .1.",
        ".9. | ... | 4..",
    },


    //
    // 5 #
    //
    {
        "8.. | ... | ...",
        "..3 | 6.. | ...",
        ".7. | .9. | 2..",

        ".5. | ..7 | ...",
        "... | .45 | 7..",
        "... | 1.. | .3.",

        "..1 | ... | ..8",
        "..8 | 5.. | .1.",
        ".9. | ... | 4..",
    },

    //
    // 6 #
    //
    // Hardcore (filled = 16)
    //
    {
        "... | ... | .13",
        ".2. | 5.. | ...",
        "... | ... | ...",
        "1.3 | ... | .7.",
        "... | 8.2 | ...",
        "..4 | ... | ...",
        "... | .4. | 5..",
        "67. | ... | 2..",
        "... | .1. | ...",
    },

    /*
    {
        ".......13",
        ".2.5.....",
        ".........",
        "1.3....7.",
        "...8.2...",
        "..4......",
        "...34.5..",
        "67....2..",
        "....1....",
    },
    *///

    /*************************************************

    // Empty board format (For user custom and copy)
    {
        "... | ... | ...",
        "... | ... | ...",
        "... | ... | ...",

        "... | ... | ...",
        "... | ... | ...",
        "... | ... | ...",

        "... | ... | ...",
        "... | ... | ...",
        "... | ... | ...",
    },

    **************************************************/
};

void make_sudoku_board(std::vector<std::vector<char>> & board, size_t index)
{
    for (size_t row = 0; row < SudokuHelper::Rows; row++) {
        std::vector<char> line;
        size_t col = 0;
        const char * prows = test_case[index].rows[row];
        char val;
        while ((val = *prows) != '\0') {
            if (val >= '0' && val <= '9') {
                if (val != '0')
                    line.push_back(val);
                else
                    line.push_back('.');
                col++;
                assert(col <= SudokuHelper::Cols);
            }
            else if (val == '.') {
                line.push_back('.');
                col++;
                assert(col <= SudokuHelper::Cols);
            }
            prows++;  
        }
        assert(col == SudokuHelper::Cols);
        board.push_back(line);
    }
}

size_t make_sudoku_board(std::vector<std::vector<char>> & board, char lines[128])
{
    size_t grid_nums = 0;
    for (size_t row = 0; row < SudokuHelper::Rows; row++) {
        std::vector<char> line;
        size_t valid = 0;
        const char * prows = &lines[row * 9];
        for (size_t col = 0; col < SudokuHelper::Cols; col++) {
            char val = *prows;
            if (val >= '0' && val <= '9') {
                if (val != '0')
                    line.push_back(val);
                else
                    line.push_back('.');
                valid++;
                assert(col <= SudokuHelper::Cols);
            }
            else if (val == '.') {
                line.push_back('.');
                valid++;
                assert(col <= SudokuHelper::Cols);
            }
            else if (val == '\0') {
                break;
            }
            prows++;  
        }
        assert(valid == SudokuHelper::Cols || valid == 0);
        board.push_back(line);
        grid_nums += valid;
    }
    return grid_nums;
}

void make_ice_sudoku_board(int sudo_in[9][9], size_t index)
{
    for (size_t row = 0; row < SudokuHelper::Rows; row++) {
        std::vector<char> line;
        size_t col = 0;
        const char * prows = test_case[index].rows[row];
        char val;
        while ((val = *prows) != '\0') {
            if (val >= '0' && val <= '9') {
                sudo_in[row][col] = val - '0';
                col++;
                assert(col <= SudokuHelper::Cols);
            }
            else if (val == '.') {
                sudo_in[row][col] = 0;
                col++;
                assert(col <= SudokuHelper::Cols);
            }
            prows++;
        }
        assert(col == SudokuHelper::Cols);
    }
}

void test_one_case(size_t index)
{
    double elapsed_time = 0.0;
    if (kEnableV1Solution)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: v1::Solution - Dancing Links\n\n");

        std::vector<std::vector<char>> board;
        make_sudoku_board(board, index);

        v1::Solution solution;
        solution.solveSudoku(board, elapsed_time);
    }

    if (kEnableV1aSolution)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: v1a::Solution - Dancing Links\n\n");

        std::vector<std::vector<char>> board;
        make_sudoku_board(board, index);

        v1a::Solution solution;
        solution.solveSudoku(board, elapsed_time);
    }

    if (kEnableV1bSolution)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: v1b::Solution - Dancing Links\n\n");

        std::vector<std::vector<char>> board;
        make_sudoku_board(board, index);

        v1b::solve_sudoku(board, elapsed_time, true);
    }

    if (kEnableV1cSolution)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: v1c::Solution - Dancing Links\n\n");

        std::vector<std::vector<char>> board;
        make_sudoku_board(board, index);

        v1c::Solution solution;
        solution.solveSudoku(board, elapsed_time);
    }

    if (kEnableV1dSolution)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: v1d::Solution - Dancing Links\n\n");

        std::vector<std::vector<char>> board;
        make_sudoku_board(board, index);

        v1d::Solution solution;
        solution.solveSudoku(board, elapsed_time);
    }

#ifdef NDEBUG
    if (kEnableV2Solution)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: v2::Solution - dfs\n\n");

        std::vector<std::vector<char>> board;
        make_sudoku_board(board, index);

        v2::Solution solution;
        solution.solveSudoku(board);
    }

    if (kEnableV3Solution)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: v3::Solution - dfs\n\n");

        std::vector<std::vector<char>> board;
        make_sudoku_board(board, index);

        v3::Solution solution;
        solution.solveSudoku(board);
    }

    if (kEnableV4Solution)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: v4::Solution - dfs\n\n");

        std::vector<std::vector<char>> board;
        make_sudoku_board(board, index);

        v4::Solution solution;
        solution.solveSudoku(board);
    }
#endif

    if (kEnableV5Solution)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: v5::Solution - dfs\n\n");

        std::vector<std::vector<char>> board;
        make_sudoku_board(board, index);

        v5::Solution solution;
        solution.solveSudoku(board);
    }

    if (kEnableV6Solution)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: v6::Solution - dfs\n\n");

        std::vector<std::vector<char>> board;
        make_sudoku_board(board, index);

        v6::Solution solution;
        solution.solveSudoku(board);
    }

    if (kEnableV7Solution)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: v7::Solution - dfs\n\n");

        std::vector<std::vector<char>> board;
        make_sudoku_board(board, index);

        v7::Solution solution;
        solution.solveSudoku(board);
    }

    if (kEnableV8Solution)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: v8::Solution - dfs\n\n");

        std::vector<std::vector<char>> board;
        make_sudoku_board(board, index);

        v8::Solution solution;
        solution.solveSudoku(board);
    }

    if (kEnableIceSudoku)
    {
        printf("--------------------------------\n\n");
        printf("SudokuSolver: ice suduku\n\n");

        ::srand((unsigned int)time(0));

        int sudo_in[9][9];
        int sudo_answer[10][9][9];
        make_ice_sudoku_board(sudo_in, index);

        jtest::StopWatch sw;
        sw.start();

        solve_sudoku(sudo_answer, sudo_in);

        sw.stop();

        print_a_sudoku(sudo_answer);

        printf("\n");
        printf("Elapsed time: %0.3f ms\n\n", sw.getElapsedMillisec());
    }

    printf("--------------------------------\n\n");
}

template <typename SolverSolution>
void test_sudoku_files_dlx(const char * filename, const char * name)
{
    printf("--------------------------------\n\n");
    printf("SudokuSolver: %s::Solution - Dancing Links\n\n", name);

    size_t puzzleCount = 0;
    double total_time = 0.0;
    SolverSolution solution;

    std::ifstream ifs;
    try {
        ifs.open(filename, std::ios::in);
        if (ifs.good()) {
            while (!ifs.eof()) {
                char line[128];
                std::memset(line, 0, 16);
                ifs.getline(line, sizeof(line) - 1);

                std::vector<std::vector<char>> board;
                size_t grid_nums = make_sudoku_board(board, line);
                if (grid_nums >= 81) {
                    double elapsed_time = 0.0;
                    bool success = solution.solveSudoku(board, elapsed_time, false);
                    total_time += elapsed_time;
                    if (success) {
                        puzzleCount++;
#ifndef NDEBUG
                        if (puzzleCount > 1000)
                            break;
#endif
                    }
                }
                else break;
            }
            ifs.close();
        }
    }
    catch (std::exception & ex) {
        std::cout << "Exception info: " << ex.what() << std::endl << std::endl;
    }

    printf("Total puzzle count = %u\n\n", (uint32_t)puzzleCount);
    printf("Total elapsed time: %0.3f ms\n\n", total_time);
}

template <typename SolverSolution>
void test_sudoku_files_dfs(const char * filename, const char * name)
{
    printf("--------------------------------\n\n");
    printf("SudokuSolver: %s::Solution - dfs\n\n", name);

    size_t puzzleCount = 0;
    double total_time = 0.0;

    std::ifstream ifs;
    try {
        ifs.open(filename, std::ios::in);
        if (ifs.good()) {
            while (!ifs.eof()) {
                char line[128];
                std::memset(line, 0, 16);
                ifs.getline(line, sizeof(line) - 1);

                std::vector<std::vector<char>> board;
                size_t grid_nums = make_sudoku_board(board, line);
                if (grid_nums >= 81) {
                    SolverSolution solution;
                    double elapsed_time = solution.solveSudoku(board, false);
                    total_time += elapsed_time;
                    puzzleCount++;
#ifndef NDEBUG
                    if (puzzleCount > 1000)
                        break;
#endif
                }
                else break;
            }
            ifs.close();
        }
    }
    catch (std::exception & ex) {
        std::cout << "Exception info: " << ex.what() << std::endl << std::endl;
    }

    printf("Total puzzle count = %u\n\n", (uint32_t)puzzleCount);
    printf("Total elapsed time: %0.3f ms\n\n", total_time);
}

void test_sudoku_files_dlx_v1b(const char * filename, const char * name)
{
    printf("--------------------------------\n\n");
    printf("SudokuSolver: %s::Solution - Dancing Links\n\n", name);

    size_t puzzleCount = 0;
    double total_time = 0.0;

    std::ifstream ifs;
    try {
        ifs.open(filename, std::ios::in);
        if (ifs.good()) {
            while (!ifs.eof()) {
                char line[128];
                std::memset(line, 0, 16);
                ifs.getline(line, sizeof(line) - 1);

                std::vector<std::vector<char>> board;
                size_t grid_nums = make_sudoku_board(board, line);
                if (grid_nums >= 81) {
                    double elapsed_time = 0.0;
                    bool success = v1b::solve_sudoku(board, elapsed_time, false);
                    total_time += elapsed_time;
                    if (success) {
                        puzzleCount++;
#ifndef NDEBUG
                        if (puzzleCount > 1000)
                            break;
#endif
                    }
                }
                else break;
            }
            ifs.close();
        }
    }
    catch (std::exception & ex) {
        std::cout << "Exception info: " << ex.what() << std::endl << std::endl;
    }

    printf("Total puzzle count = %u\n\n", (uint32_t)puzzleCount);
    printf("Total elapsed time: %0.3f ms\n\n", total_time);
}

int main(int argc, char * argv[])
{
    const char * filename = nullptr;
    const char * out_file = nullptr;
    if (argc > 2) {
        filename = argv[1];
        out_file = argv[2];
    }
    else if (argc > 1) {
        filename = argv[1];
    }

    jtest::CPU::warmup(1000);

    if (1)
    {
        if (filename == nullptr) {
            test_one_case(TEST_CASE_INDEX);
        }
    }

    if (1)
    {
        if (out_file != nullptr) {
            v1b::run_test_sudoku17(filename, out_file);
        }
        else if (filename != nullptr) {
            //test_sudoku_files_dlx<v1a::Solution>(filename, "v1a");
            test_sudoku_files_dlx_v1b(filename, "v1b");
            test_sudoku_files_dlx<v1c::Solution>(filename, "v1c");
            test_sudoku_files_dlx<v1d::Solution>(filename, "v1d");
            //test_sudoku_files_dfs<v5::Solution>(filename, "v5");
            //test_sudoku_files_dfs<v6::Solution>(filename, "v6");
            test_sudoku_files_dfs<v8::Solution>(filename, "v8");
        }
    }

#if !defined(NDEBUG) && defined(_MSC_VER)
    ::system("pause");
#endif

    return 0;
}
