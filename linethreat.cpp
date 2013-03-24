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
#include <QtGlobal>

// PieceCoords
    // Public:
        PieceCoords::PieceCoords(const int& c, const int& r)
        : col(c), row(r) {}

        bool PieceCoords::isValid() const
        { return col >= 0 && col < 7 && row >= 0 && row < 6; }

        bool PieceCoords::operator==(const PieceCoords& other) const
        { return col == other.col && row == other.row; }

        bool PieceCoords::operator!=(const PieceCoords& other) const
        { return col != other.col || row != other.row; }

// LineThreat
    // Public:
        LineThreat::LineThreat(const int& startCol, const int& startRow, const Direction& dir, const int& threatLevel)
            : dir(dir), solved(false), solvedTemp(false), start(startCol, startRow), threatLevel(qMin(3, qMax(0, threatLevel)))
        {}

        LineThreat::LineThreat(const PieceCoords& startCoords, const Direction& dir, const int& threatLevel)
        : dir(dir), solved(false), solvedTemp(false), start(startCoords), threatLevel(qMin(3, qMax(0, threatLevel)))
        {}

        bool LineThreat::operator==(const LineThreat& other) const
        { return dir == other.dir && start == other.start; }

        PieceCoords LineThreat::operator[](const unsigned int& n) const
        { return at(n); }
        PieceCoords LineThreat::at(const unsigned int& n) const
        {
            if(n == 0) return start;

            const int deltaCol = dir == Vertical ? 0 : (dir == DiagonalLeft ? -1 : 1);
            const int deltaRow = dir == Horizontal ? 0 : -1;

            if(n >= 3) return PieceCoords(start.col + deltaCol * 3, start.row + deltaRow * 3);
            return PieceCoords(start.col + deltaCol * n, start.row + deltaRow * n);
        }
        const PieceCoords& LineThreat::startCoords() const
        { return start; }

        bool LineThreat::coversCoords(const int& col, const int& row)
        {
            switch(dir)
            {
                case Horizontal:    return row == start.row && col >= start.col && col < start.col + 4;
                case Vertical:      return col == start.col && row <= start.row && row > start.row - 4;
                case DiagonalRight: return col >= start.col && col < start.col + 4 && col - start.col == start.row - row;
                case DiagonalLeft:  return col <= start.col && col > start.col - 4 && start.col - col == start.row - row;
            }
            return false;
        }

        bool LineThreat::coversCol(const int& col)
        {
            switch(dir)
            {
                case Horizontal:    return col >= start.col && col < start.col + 4;
                case Vertical:      return col == start.col;
                case DiagonalRight: return col >= start.col && col < start.col + 4;
                case DiagonalLeft:  return col > start.col - 4 && col <= start.col;
            }
            return false;
        }

        int LineThreat::mostLeftCol() const
        {
            switch(dir)
            {
                case Horizontal:
                case Vertical:
                case DiagonalRight: return start.col;
                case DiagonalLeft:  return start.col - 3;
            }
            return 0;
        }
        int LineThreat::mostRightCol() const
        {
            switch(dir)
            {
                case Vertical:
                case DiagonalLeft:  return start.col;
                case Horizontal:
                case DiagonalRight: return start.col + 4;
            }
            return 0;
        }

        int LineThreat::level() const
        { return threatLevel; }
        void LineThreat::setLevel(const int& newThreatLevel)
        { threatLevel = qMin(3, qMax(0, newThreatLevel)); }
