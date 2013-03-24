/************************************************************************
* This file is part of IntelliCon.                                      *
*                                                                       *
* IntelliCon is free software: you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation, either version 3 of the License, or     *
* (at your option) any later version.                                   *
*                                                                       *
* IntelliCon is distributed in the hope that it will be useful,         *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
* GNU General Public License for more details.                          *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with IntelliCon.  If not, see <http://www.gnu.org/licenses/>.   *
************************************************************************/

#ifndef MOVESIMULATOR_H
#define MOVESIMULATOR_H

#include "boardext.h"
#include <QRunnable>
#include <QThreadPool>

// Indicates how smart/good a move would be
enum MoveSmartness
{
    Unknown         = 0,    // The value isn't set yet
    Impossible      = 1,    // It's impossible to play in this column
    DirectLose      = 2,    // Playing in this column means a direct win for the opponent
    NotAllSolved    = 3,    // Not all of the opponent's threats can be solved when we play in this column
    NeedsTreeSearch = 4,    // If a tree search is needed to evaluate this move
    AllSolved       = 5,    // All of the opponent's threats can be solved when we play in this column
    AllSolvedWin    = 6     // All of the opponent's threats can be solved, and we win the game using this solution
};
Q_DECLARE_METATYPE(MoveSmartness)

class MoveSimulator : public QObject, public QRunnable
{
    Q_OBJECT

    public:
        MoveSimulator(const Board& board, const int& move, const bool& isRed, const MoveSmartness& leastAchievement = AllSolvedWin);

        void run();

        void setInterruptedPointer(const bool* p);

        void dontTryYellowSolve();

    public slots:
        MoveSmartness simulate();

    signals:
        void done(const int& move, const MoveSmartness& result);
        
    private:
        BoardExt board;                 // The board (extended version that can search for threats etc)
        int move;                       // The move that should be simulated
        bool isRed;                     // The color of the player
        const bool* keepRunning;        // Whether we should keep searching for moves (true) or are interrupted (false)

        MoveSmartness leastAchievement; // What we try to achieve at least

        bool tryYellowSolve;            // Whether red should consult yellow if he can't solve the board himself

        // Searches solutions for the given move
        MoveSmartness findSolutions(const int& move);
        // Recursive function to find a set of solutions that solve all problems
        MoveSmartness findSolutionSet(std::list<LineThreat*> threats, const std::list<ThreatSolution*>& solutions);
};

#endif // MOVESIMULATOR_H
