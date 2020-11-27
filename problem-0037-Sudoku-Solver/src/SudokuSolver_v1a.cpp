
#include "SudokuSolver_v1a.h"

#include <stdio.h>
#include <stdlib.h>

#include "StopWatch.h"

using namespace LeetCode::Problem_37::v1a;

size_t DancingLinks::recur_counter = 0;

template <typename TDancingLinksX>
void SudokuSolver::display_answer(std::vector<std::vector<char>> & board,
                                  const TDancingLinksX * dancingLinks) {
    for (auto idx : dancingLinks->get_answer()) {
        if (idx > 0) {
            board[rows[idx]][cols[idx]] = (char)numbers[idx] + '1';
        }
    }

    SudokuHelper::display_board(board);
}

template <typename TDancingLinksX>
void SudokuSolver::display_answers(std::vector<std::vector<char>> & board,
                                   const TDancingLinksX * dancingLinks) {
    printf("Total answers: %d\n\n", (int)dancingLinks->get_answers().size());
    int i = 0;
    for (auto answer : dancingLinks->get_answers()) {
        for (auto idx : answer) {
            board[rows[idx]][cols[idx]] = (char)numbers[idx] + '1';
        }
        SudokuHelper::display_board(board, false, i);
        i++;
    }
}

bool Solution::solveSudoku(std::vector<std::vector<char>> & board,
                           double & elapsed_time,
                           bool verbose)
{
    if (verbose) {
        SudokuHelper::display_board(board, true);
    }

    size_t recur_counter = 0;

    jtest::StopWatch sw;
    sw.start();

    SudokuSolver solver(board);

#if 1
    DancingLinks dancingLinks(solver.getDlkMatrix(), (SudokuHelper::TotalSize + 1) * 4 + 1);
    //bool success = dancingLinks.solve_non_recursive();
    bool success = dancingLinks.solve(recur_counter);
#else
    DLX dancingLinks(solver.getDlkMatrix(), SudokuHelper::TotalSize * 4 + 1);
    bool success = dancingLinks.solve();
#endif

    sw.stop();
    elapsed_time = sw.getElapsedMillisec();

    if (verbose) {
        if (kSearchAllAnswers)
            solver.display_answers(board, &dancingLinks);
        else
            solver.display_answer(board, &dancingLinks);
        printf("Elapsed time: %0.3f ms, recur_counter: %u\n\n",
               elapsed_time, (uint32_t)recur_counter);
    }

    return success;
}
