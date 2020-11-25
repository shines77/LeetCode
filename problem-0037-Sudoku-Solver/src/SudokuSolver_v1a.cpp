
#include "SudokuSolver_v1a.h"

#include <stdio.h>
#include <stdlib.h>

#include "StopWatch.h"

using namespace LeetCode::Problem_37::v1a;

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

double Solution::solveSudoku(std::vector<std::vector<char>> & board, bool verbose)
{
    double elapsed_time;
    if (verbose) {
        SudokuHelper::display_board(board, true);
    }

    jtest::StopWatch sw;
    sw.start();

    SudokuSolver solver(board);

#if 1
    DancingLinks dancingLinks(solver.getDlkMatrix(), SudokuSolver::TotalSize * 4 + 1);
    //dancingLinks.solve_non_recursive();
    dancingLinks.solve();
#else
    DLX dancingLinks(solver.getDlkMatrix(), SudokuSolver::TotalSize * 4 + 1);
    dancingLinks.solve();
#endif

    sw.stop();
    elapsed_time = sw.getElapsedMillisec();

    if (verbose) {
        if (kSearchAllStages)
            solver.display_answers(board, &dancingLinks);
        else
            solver.display_answer(board, &dancingLinks);
        printf("Elapsed time: %0.3f ms\n\n", elapsed_time);
    }

    return elapsed_time;
}
