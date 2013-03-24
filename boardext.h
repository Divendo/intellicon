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

#ifndef BOARDEXT_H
#define BOARDEXT_H

#include "gameboard.h"
#include "linethreat.h"
#include <QMutex>
#include <list>

class BoardExt : public Board, public QMutex
{
    public:
        BoardExt(const bool& playerIsRed);
        virtual ~BoardExt();

        // Copy
        // Doesn't copy the solutions or threats
        BoardExt(const Board& other, const bool& playerIsRed);

        void changesMade(const Board& b);

        // Finds out which columns are playable and if they are playable which row is directly playable
        void findPlayableCols();
        // Returns which row is directly playable in the given column, -1 if the column is full
        int playableRow(const int& col) const;

        // Finds all of the opponent's threats
        void searchForThreats();
        std::list<LineThreat*> threats;
        const std::list<LineThreat*>& threatsAt(const int& col, const int& row) const;
        const std::list<LineThreat*>& threatsAt(const PieceCoords& coords) const;
        // Convenience function: checks if a threat of level 3 is found at the given position
        bool hasLevel3Threat(const int& col, const int& row) const;

        // Finds all of our threats
        void searchForWinningThreats();
        std::list<LineThreat*> winningThreats;
        const std::list<LineThreat*>& winningThreatsAt(const int& col, const int& row) const;
        const std::list<LineThreat*>& winningThreatsAt(const PieceCoords& coords) const;
        // Convenience function: checks if a winning threat of level 3 is found at the given position
        bool hasLevel3WinningThreat(const int& col, const int& row) const;

        // Whether red has an odd threat or not
        bool hasOddThreat();

        // Solves the enemy threats that are solved by our odd threats
        void solveByOddThreats();

        // Plays a piece in the given column and returns the new board
        Board doMove(const int& col, const Piece& piece) const;

        // Returns the number of pieces on the board (assumes that findPlayableCols() is called before)
        int pieceCount() const;

        // Searches for all possible solutions to every possible threat and lists them in 'solutions'
        void searchSolutions();
        std::list<ThreatSolution*> solutions;

        bool isRed;                     // Whether the player is red or not

    private:
        typedef std::vector< std::list<LineThreat*> > ThreatBoardCol;
        typedef std::vector< ThreatBoardCol > ThreatBoard;
        std::vector<int> playableCols;  // The playable columns, the vector has the following format:
                                        //   playableCols[column]    =   if not playable: -1, else the row that's playable in this column
        ThreatBoard threatBoard;        // The opponent's threats on the board, per square
        ThreatBoard winningThreatBoard; // Our threats on the board, per square

        int oddThreatCol1;                              // The first column that shouldn't be included in the search for solutions
        int oddThreatCol2;                              // The second column that shouldn't be included in the search for solutions
                                                        // Both are -1 if no odd threat is found or when the player's color is yellow
        int oddThreatRow1;                              // The row of the odd threat in oddThreatCol1
        int oddThreatRow2;                              // The row of the lowest threat in oddThreatCol2 (if used)

        static const char SC1;                          // Solution combination 1: allowed if sets of squares are disjoint
        static const char SC2;                          // Solution combination 2: allowed if no ClaimEven below the inverse
        static const char SC3;                          // Solution combination 3: allowed if sets of squares are column wise disjoint or equal.
        static const char SC4;                          // Solution combination 4: allowed if the sets of squares are disjoint,
                                                        //      and the set of columns used by the inverses are disjoint or equal.
        static const char solutionCombinations[11][11];

        bool findThreatsFromPoint(const int& col, const int& row, const LineThreat::Direction& dir);
        bool findWinningThreatsFromPoint(const int& col, const int& row, const LineThreat::Direction& dir);

        // Functions that find all possible solutions
        void findClaimEvens();
        void findBaseInverses();
        void findVerticals();
        void findAfterEvens();
        void findLowInverses();
        void findHighInverses();
        void findBaseClaims();
        void findBefores();
        void findSpecialBefores();
        void findAfterBaseInverses();
        void findAfterVerticals();

        // Functions that check whether two solutions can be combined
        bool solutionCanCombine1(const ThreatSolution* s1, const ThreatSolution* s2) const;
        bool solutionCanCombine2(const ThreatSolution* s1, const ThreatSolution* s2) const;
        bool solutionCanCombine3(const ThreatSolution* s1, const ThreatSolution* s2) const;
        bool solutionCanCombine4(const ThreatSolution* s1, const ThreatSolution* s2) const;
};

#endif // BOARDEXT_H
