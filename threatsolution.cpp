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

#include "linethreat.h"
#include <algorithm>

// ThreatSolution:
    // Public:
        ThreatSolution::ThreatSolution(const Type& type)
        : type(type), dontUse(false), win(false)
        {
            switch(type)
            {
                case ClaimEven:         squares.reserve(2); break;
                case BaseInverse:       squares.reserve(2); break;
                case Vertical:          squares.reserve(2); break;
                case AfterEven:         squares.reserve(8); break;
                case LowInverse:        squares.reserve(4); break;
                case HighInverse:       squares.reserve(6); break;
                case BaseClaim:         squares.reserve(4); break;
                case Before:            squares.reserve(8); break;
                case SpecialBefore:     squares.reserve(9); break;
                case AfterBaseInverse:  squares.reserve(2); break;
                case AfterVertical:     squares.reserve(2); break;
            }

            // Some types will always win the game
            if(type == AfterEven || type == AfterBaseInverse || type == AfterVertical)
                makeGameWinner();
        }

        bool ThreatSolution::operator==(const ThreatSolution& other) const
        {
            if(other.type == Before || other.type == SpecialBefore)
                return other.operator==(*this);

            if(type != other.type || squares.size() != other.squares.size()) return false;

            for(unsigned int i = 0; i < squares.size(); ++i)
            {
                if(squares[i] != other[i]) return false;
            }

            return true;
        }

        //bool ThreatSolution::solves(const LineThreat& threat) const
        //{ return std::find_if(solvedThreats.begin(), solvedThreats.end(), LineThreatEqualToPointer(threat)) != solvedThreats.end(); }

        const PieceCoords& ThreatSolution::operator[](const unsigned int& n) const
        { return squares[n]; }

        const PieceCoords& ThreatSolution::at(const unsigned int& n) const
        { return squares[n]; }

        unsigned int ThreatSolution::squareCount() const
        { return squares.size(); }

        void ThreatSolution::addSquare(const PieceCoords& square)
        {
            for(std::vector<PieceCoords>::iterator pos = squares.begin(); pos != squares.end(); ++pos)
            {
                if(square.col < pos->col)
                {
                    squares.insert(pos, square);
                    return;
                }
                else if(square.col == pos->col)
                {
                    if(square.row < pos->row)
                    {
                        squares.insert(pos, square);
                        return;
                    }
                    else if(square.row == pos->row)
                        return;
                }
            }

            // The square wasn't inserted, so we add it to the back
            squares.push_back(square);
        }
        void ThreatSolution::addSquare(const int& col, const int& row)
        { addSquare(PieceCoords(col, row)); }

        bool ThreatSolution::winsGame() const
        { return win; }
        void ThreatSolution::makeGameWinner()
        { win = true; }

        bool ThreatSolution::interferes(const ThreatSolution* other) const
        {
            std::vector<PieceCoords>::const_iterator pos = squares.begin();
            std::vector<PieceCoords>::const_iterator pos2 = other->squares.begin();

            while(pos != squares.end() && pos2 != other->squares.end())
            {
                if(pos->col == pos2->col)
                {
                    if(pos->row == pos2->row)       return true;
                    else if(pos->row < pos2->row)   ++pos;
                    else                            ++pos2;
                }
                else if(pos->col < pos2->col)
                    ++pos;
                else
                    ++pos2;
            }

            return false;
        }

        bool ThreatSolution::interferesColumnWise(const ThreatSolution* other) const
        {
            // Check if one of the 2 solutions is a Before or SpecialBefore and can therefore have special squares
            const ThreatSolutionBefore* thisBefore = dynamic_cast<const ThreatSolutionBefore*>(this);
            const ThreatSolutionBefore* otherBefore = dynamic_cast<const ThreatSolutionBefore*>(other);

            // Which columns are used by this solution
            bool colUsed[7] = {false, false, false, false, false, false, false};
            // Which squares are used by this solution
            bool squareUsed[7][6] = {{false, false, false, false, false, false},
                                     {false, false, false, false, false, false},
                                     {false, false, false, false, false, false},
                                     {false, false, false, false, false, false},
                                     {false, false, false, false, false, false},
                                     {false, false, false, false, false, false},
                                     {false, false, false, false, false, false}};

            // Fill the colUsed and squareUsed arrays
            for(std::vector<PieceCoords>::const_iterator pos = squares.begin(); pos != squares.end(); ++pos)
            {
                colUsed[pos->col] = true;
                squareUsed[pos->col][pos->row] = true;
            }

            // Check the filled arrays against the squares of the other solution
            int currCol = -1;
            bool squaresDisjoint = true;
            bool squaresEqual = true;
            for(std::vector<PieceCoords>::const_iterator pos = other->squares.begin(); pos != other->squares.end(); ++pos)
            {
                if(!colUsed[pos->col]) continue;

                if(pos->col != currCol)
                {
                    if(!squaresDisjoint && !squaresEqual) return true;
                    currCol = pos->col;
                    squaresDisjoint = true;
                    squaresEqual = true;
                    if(thisBefore != 0 && thisBefore->isSpecialSquareColumn(currCol))
                        squaresEqual = false;
                    if(otherBefore != 0 && otherBefore->isSpecialSquareColumn(currCol))
                        squaresEqual = false;
                }

                if(!squareUsed[pos->col][pos->row]) squaresEqual = false;
                else                                squaresDisjoint = false;
            }

            return !squaresDisjoint && !squaresEqual;
        }

// ThreatSolutionBefore:
    // Public:
        ThreatSolutionBefore::ThreatSolutionBefore(const Type& type)
        : ThreatSolution(type), claimEvenUsed(7, false), specialSquareCols(7, false) {}

        bool ThreatSolutionBefore::operator==(const ThreatSolution& other) const
        {
            if(!ThreatSolution::operator==(other)) return false;

            const ThreatSolutionBefore* before = dynamic_cast<const ThreatSolutionBefore*>(&other);
            if(before == 0) return false;

            for(unsigned int col = 0; col < 7;  ++col)
            {
                if(claimEvenUsed[col] != before->claimEvenUsed[col]) return false;
            }

            return true;
        }

        void ThreatSolutionBefore::useClaimEvenIn(const int& col)
        { claimEvenUsed[col] = true; }
        bool ThreatSolutionBefore::claimEvenUsedInCol(const int& col) const
        { return claimEvenUsed[col]; }

        int ThreatSolutionBefore::lowestUsedSquareInCol(const int& col) const
        {
            for(std::vector<PieceCoords>::const_iterator pos = squares.begin(); pos != squares.end(); ++pos)
            {
                if(pos->col == col) return pos->row;
            }

            return -1;
        }

        void ThreatSolutionBefore::addSpecialSquareColumn(const int& col)
        { specialSquareCols[col] = true; }
        bool ThreatSolutionBefore::isSpecialSquareColumn(const int& col) const
        { return specialSquareCols[col]; }
