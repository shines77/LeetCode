
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

#include "BitSet.h"
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

    static const size_t TotalCellLiterals = Rows * Cols;
    static const size_t TotalRowLiterals = Rows * Numbers;
    static const size_t TotalColLiterals = Cols * Numbers;
    static const size_t TotalBoxLiterals = Palaces * Numbers;

    static const size_t TotalLiterals =
        TotalCellLiterals + TotalRowLiterals + TotalColLiterals + TotalBoxLiterals;

    static const size_t LiteralFirst     = 0;
    static const size_t CellLiteralFirst = LiteralFirst;
    static const size_t RowLiteralFirst  = CellLiteralFirst + TotalCellLiterals;
    static const size_t ColLiteralFirst  = RowLiteralFirst + TotalRowLiterals;
    static const size_t BoxLiteralFirst  = ColLiteralFirst + TotalColLiterals;
    static const size_t LiteralLast      = BoxLiteralFirst + TotalBoxLiterals;

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
