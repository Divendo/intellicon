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

#ifndef LINETHREAD_H
#define LINETHREAD_H

#include <vector>
#include <list>

class ThreatSolution;

class PieceCoords
{
    public:
        PieceCoords(const int& c = 0, const int& r = 0);

        int col;
        int row;

        bool isValid() const;
        bool operator==(const PieceCoords& other) const;
        bool operator!=(const PieceCoords& other) const;
};

class LineThreat
{
    public:
        enum Direction
        { Vertical, Horizontal, DiagonalRight, DiagonalLeft };
        /* Directions are as follows:
         *  Vertical:       vertically down
         *  Horizontal:     horizontally to the right
         *  DiagonalRight:  diagonally down to the right
         *  DiagonalLeft:   diagonally down to the left */

        LineThreat(const int& startCol, const int& startRow, const Direction& dir, const int& threatLevel);
        LineThreat(const PieceCoords& startCoords, const Direction& dir, const int& threatLevel);

        Direction dir;

        bool operator==(const LineThreat& other) const;

        PieceCoords operator[](const unsigned int& n) const;
        PieceCoords at(const unsigned int& n) const;
        const PieceCoords& startCoords() const;
        bool coversCoords(const int& col, const int& row);
        bool coversCol(const int& col);

        int mostLeftCol() const;
        int mostRightCol() const;

        int level() const;
        void setLevel(const int& newThreatLevel);

        std::list<ThreatSolution*> solutions;

        bool solved;            // Whether this threat is solved or not
        bool solvedTemp;        // Temporarily marks this threat as solved

    private:
        PieceCoords start;
        int threatLevel;
};

class ThreatSolution
{
    public:
        enum Type
        {
            ClaimEven       = 0,
            BaseInverse     = 1,
            Vertical        = 2,
            AfterEven       = 3,
            LowInverse      = 4,
            HighInverse     = 5,
            BaseClaim       = 6,
            Before          = 7,
            SpecialBefore   = 8,
            AfterBaseInverse= 9,
            AfterVertical   = 10
        };

        ThreatSolution(const Type& type);

        Type type;
        std::vector<LineThreat*> solvedThreats;

        std::vector<ThreatSolution*> cantCombine;

        virtual bool operator==(const ThreatSolution& other) const;

        // The squares are sorted by column first and then by row, in ascending order
        const PieceCoords& operator[](const unsigned int& n) const;
        const PieceCoords& at(const unsigned int& n) const;
        unsigned int squareCount() const;
        void addSquare(const PieceCoords& square);
        void addSquare(const int& col, const int& row);

        // Returns true if this solution will win the game
        bool winsGame() const;
        // Make this solution a game winner
        void makeGameWinner();

        // Whether the set of squares used by this solution interferes with the set of squares of the other solution
        bool interferes(const ThreatSolution* other) const;
        // Check per column whether the squares used by this solution interfere with the set of squares used by the other solution
        // The squares in a column are not said to interfere if exaclty the same squares are used by both solutions in that column
        bool interferesColumnWise(const ThreatSolution* other) const;

        bool dontUse;

    protected:
        std::vector<PieceCoords> squares;
        bool win;
};

class ThreatSolutionBefore : public ThreatSolution
{
    public:
        ThreatSolutionBefore(const Type& type);

        bool operator==(const ThreatSolution& other) const;

        void useClaimEvenIn(const int& col);
        bool claimEvenUsedInCol(const int& col) const;

        int lowestUsedSquareInCol(const int& col) const;

        void addSpecialSquareColumn(const int& col);
        bool isSpecialSquareColumn(const int& col) const;

    private:
        std::vector<bool> claimEvenUsed;        // In which columns a ClaimEven is used
        std::vector<bool> specialSquareCols;    // In which columns special squares are used, and therefore in these columns the set of squares must be disjoint
};

#endif // LINETHREAD_H
