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

#include "boardext.h"
#include <cstdlib>
#include <cmath>

// Public:
    BoardExt::BoardExt(const bool& playerIsRed)
    : isRed(playerIsRed), oddThreatCol1(-1), oddThreatCol2(-1) { }

    BoardExt::~BoardExt()
    {
        for(std::list<LineThreat*>::iterator pos = threats.begin(); pos != threats.end(); ++pos)
            delete *pos;
        for(std::list<LineThreat*>::iterator pos = winningThreats.begin(); pos != winningThreats.end(); ++pos)
            delete *pos;
        for(std::list<ThreatSolution*>::iterator pos = solutions.begin(); pos != solutions.end(); ++pos)
            delete *pos;
    }

    BoardExt::BoardExt(const Board& other, const bool& playerIsRed)
    : Board(other), isRed(playerIsRed), oddThreatCol1(-1), oddThreatCol2(-1)
    { }

    void BoardExt::changesMade(const Board& b)
    {
        clear();

        reserve(b.size());
        for(unsigned int col = 0; col < b.size(); ++col)
        {
            std::vector<Piece> tmp;
            tmp.reserve(b[col].size());
            for(unsigned int row = 0; row < b[col].size(); ++row)
                tmp.push_back(b[col][row]);
            push_back(tmp);
        }
    }

    void BoardExt::findPlayableCols()
    {
        playableCols.clear();
        playableCols.reserve(size());
        for(unsigned int col = 0; col < size(); ++col)
        {
            int row = 6;
            while(row > 0 && at(col)[row - 1] == Empty) --row;

            if(row == 6)    playableCols.push_back(-1);
            else            playableCols.push_back(row);
        }
    }

    int BoardExt::playableRow(const int& col) const
    { return playableCols[col]; }

    void BoardExt::searchForThreats()
    {
        for(std::list<LineThreat*>::iterator pos = threats.begin(); pos != threats.end(); ++pos)
            delete *pos;
        threats.clear();
        threatBoard.clear();
        threatBoard.resize(7, ThreatBoardCol(6, std::list<LineThreat*>()));

        const Piece ownColor = isRed ? Red : Yellow;
        for(unsigned int col = 0; col < 7; ++col)
        {
            for(unsigned int row = 0; row < 6; ++row)
            {
                if(at(col)[row] != ownColor)
                {
                    findThreatsFromPoint(col, row, LineThreat::Vertical);
                    findThreatsFromPoint(col, row, LineThreat::Horizontal);
                    findThreatsFromPoint(col, row, LineThreat::DiagonalRight);
                    findThreatsFromPoint(col, row, LineThreat::DiagonalLeft);
                }
            }
        }
    }
    const std::list<LineThreat*>& BoardExt::threatsAt(const int& col, const int& row) const
    { return threatBoard[col][row]; }
    const std::list<LineThreat*>& BoardExt::threatsAt(const PieceCoords& coords) const
    { return threatBoard[coords.col][coords.row]; }

    bool BoardExt::hasLevel3Threat(const int& col, const int& row) const
    {
        const std::list<LineThreat*>& threatsAtThisSquare = threatBoard[col][row];
        for(std::list<LineThreat*>::const_iterator pos = threatsAtThisSquare.begin(); pos != threatsAtThisSquare.end(); ++pos)
        {
            if((*pos)->level() == 3)
                return true;
        }
        return false;
    }

    void BoardExt::searchForWinningThreats()
    {
        for(std::list<LineThreat*>::iterator pos = winningThreats.begin(); pos != winningThreats.end(); ++pos)
            delete *pos;
        winningThreats.clear();
        winningThreatBoard.clear();
        winningThreatBoard.resize(7, ThreatBoardCol(6, std::list<LineThreat*>()));

        const Piece enemyColor = isRed ? Yellow : Red;
        for(unsigned int col = 0; col < 7; ++col)
        {
            for(unsigned int row = 0; row < 6; ++row)
            {
                if(at(col)[row] != enemyColor)
                {
                    findWinningThreatsFromPoint(col, row, LineThreat::Vertical);
                    findWinningThreatsFromPoint(col, row, LineThreat::Horizontal);
                    findWinningThreatsFromPoint(col, row, LineThreat::DiagonalRight);
                    findWinningThreatsFromPoint(col, row, LineThreat::DiagonalLeft);
                }
            }
        }
    }
    const std::list<LineThreat*>& BoardExt::winningThreatsAt(const int& col, const int& row) const
    { return winningThreatBoard[col][row]; }
    const std::list<LineThreat*>& BoardExt::winningThreatsAt(const PieceCoords& coords) const
    { return winningThreatBoard[coords.col][coords.row]; }

    bool BoardExt::hasLevel3WinningThreat(const int& col, const int& row) const
    {
        const std::list<LineThreat*>& winningAtThisSquare = winningThreatBoard[col][row];
        for(std::list<LineThreat*>::const_iterator pos = winningAtThisSquare.begin(); pos != winningAtThisSquare.end(); ++pos)
        {
            if((*pos)->level() == 3)
                return true;
        }
        return false;
    }

    bool BoardExt::hasOddThreat()
    {
        // Reset the odd threat columns
        oddThreatCol1 = -1;
        oddThreatCol2 = -1;

        // Check for odd threats, and if we have one we mark these columns as unusable for the solutions
        if(isRed)
        {
            PieceCoords coords;
            for(std::list<LineThreat*>::const_iterator pos = winningThreats.begin(); pos != winningThreats.end(); ++pos)
            {
                if((*pos)->level() == 3)
                {
                    for(int i = 0; i < 4; ++i)
                    {
                        coords = (*pos)->at(i);
                        if(at(coords.col)[coords.row] == Empty && playableCols[coords.col] != coords.row)
                        {
                            if(coords.row % 2 == 0)
                            {
                                oddThreatCol1 = coords.col;
                                oddThreatRow1 = coords.row;
                                return true;
                            }
                            break;
                        }
                    }
                }
                else if((*pos)->level() == 2)
                {
                    PieceCoords empty1(-1, -1);
                    PieceCoords empty2(-1, -1);
                    for(int i = 0; i < 4; ++i)
                    {
                        coords = (*pos)->at(i);
                        if(at(coords.col)[coords.row] == Empty)
                        {
                            if(empty1.col == -1)
                                empty1 = coords;
                            else
                            {
                                empty2 = coords;
                                break;
                            }
                        }
                    }

                    if(empty1.row % 2 != 0 || empty2.row % 2 != 0) continue;

                    if(playableCols[empty1.col] != empty1.row)
                    {
                        std::list<LineThreat*> winningAtThisSquare = winningThreatBoard[empty1.col][empty1.row];
                        for(std::list<LineThreat*>::const_iterator pos2 = winningAtThisSquare.begin(); pos2 != winningAtThisSquare.end(); ++pos2)
                        {
                            if((*pos2)->level() != 2) continue;

                            for(int j = 0; j < 4; ++j)
                            {
                                coords = (*pos2)->at(j);
                                if(empty1 == coords || at(coords.col)[coords.row] != Empty) continue;

                                if(coords.col == empty2.col && std::abs(coords.row - empty2.row) == 1)
                                {
                                    oddThreatCol1 = empty1.col;
                                    oddThreatRow1 = empty1.row;
                                    oddThreatCol2 = empty2.col;
                                    oddThreatRow2 = std::min(empty2.row, coords.row);
                                    return true;
                                }
                                break;
                            }
                        }
                    }
                    if(playableCols[empty2.col] != empty2.row)
                    {
                        std::list<LineThreat*> winningAtThisSquare = winningThreatBoard[empty2.col][empty2.row];
                        for(std::list<LineThreat*>::const_iterator pos2 = winningAtThisSquare.begin(); pos2 != winningAtThisSquare.end(); ++pos2)
                        {
                            if((*pos2)->level() != 2) continue;

                            for(int j = 0; j < 4; ++j)
                            {
                                coords = (*pos2)->at(j);
                                if(empty2 == coords || at(coords.col)[coords.row] != Empty) continue;

                                if(coords.col == empty1.col && std::abs(coords.row - empty1.row) == 1)
                                {
                                    oddThreatCol1 = empty2.col;
                                    oddThreatRow1 = empty2.row;
                                    oddThreatCol2 = empty1.col;
                                    oddThreatRow2 = std::min(empty1.row, coords.row);
                                    return true;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    void BoardExt::solveByOddThreats()
    {
        std::list<LineThreat*> possibleSolves;
        if(oddThreatCol1 != -1)
        {
            for(int row = playableCols[oddThreatCol1] + 1 - playableCols[oddThreatCol1] % 2; row < 6; ++row)
            {
                if(row % 2 == 0)
                {
                    const std::list<LineThreat*>& solved = threatBoard[oddThreatCol1][row];
                    for(std::list<LineThreat*>::const_iterator pos = solved.begin(); pos != solved.end(); ++pos)
                        (*pos)->solved = true;
                }
                else if(row > oddThreatRow1)
                    possibleSolves.insert(possibleSolves.end(), threatBoard[oddThreatCol1][row].begin(), threatBoard[oddThreatCol1][row].end());
            }
        }

        if(oddThreatCol2 == -1)
        {
            for(std::list<LineThreat*>::const_iterator pos = possibleSolves.begin(); pos != possibleSolves.end(); ++pos)
                (*pos)->solved = true;
        }
        else
        {
            for(std::list<LineThreat*>::const_iterator pos = possibleSolves.begin(); pos != possibleSolves.end(); ++pos)
            {
                for(int row = oddThreatRow2 + 1; row < 6; ++row)
                {
                    if((*pos)->coversCoords(oddThreatCol2, row))
                    {
                        (*pos)->solved = true;
                        break;
                    }
                }
            }

            // Even threat above the odd threat
            if(oddThreatRow2 == oddThreatRow1)
            {
                // Yellow will not get both the square above the crossing square and the odd square in the other column
                const std::list<LineThreat*>& posSolves = threatBoard[oddThreatCol1][oddThreatRow1 + 1];
                for(std::list<LineThreat*>::const_iterator pos = posSolves.begin(); pos != posSolves.end(); ++pos)
                {
                    if((*pos)->coversCoords(oddThreatCol2, oddThreatRow2 - 1))
                        (*pos)->solved = true;
                }

                // If the odd square in the other column is playable, the highest square in the crossing column which
                // can be taken by yellow, is the square directly above the crossing square
                if(playableCols[oddThreatCol2] == oddThreatRow2 - 1)
                {
                    for(int row = oddThreatRow1 + 2; row < 6; ++row)
                    {
                        const std::list<LineThreat*>& solved = threatBoard[oddThreatCol1][row];
                        for(std::list<LineThreat*>::const_iterator pos = solved.begin(); pos != solved.end(); ++pos)
                            (*pos)->solved = true;
                    }
                }
            }

            // If the first empty square in the crossing column is odd and the odd square in the other column is
            // not directly playable, a Baseinverse can be used on the lowest squares of both columns, giving one of
            // the two squares to red
            if(playableCols[oddThreatCol2] < oddThreatRow2 && playableCols[oddThreatCol1] % 2 == 0)
            {
                const std::list<LineThreat*>& posSolves = threatBoard[oddThreatCol1][playableCols[oddThreatCol1]];
                for(std::list<LineThreat*>::const_iterator pos = posSolves.begin(); pos != posSolves.end(); ++pos)
                {
                    if((*pos)->coversCoords(oddThreatCol2, playableCols[oddThreatCol2]))
                        (*pos)->solved = true;
                }
            }
        }
    }

    Board BoardExt::doMove(const int& col, const Piece& piece) const
    {
        Board b = *this;
        if(playableCols[col] == -1) return b;
        b[col][playableCols[col]] = piece;
        return b;
    }

    int BoardExt::pieceCount() const
    {
        int out = 0;
        for(int col = 0; col < 7; ++col)
        {
            if(playableCols[col] != -1) out += playableCols[col];
            else                        out += 6;
        }
        return out;
    }

    void BoardExt::searchSolutions()
    {
        // Clear all solutions
        for(std::list<LineThreat*>::iterator pos = threats.begin(); pos != threats.end(); ++pos)
            (*pos)->solutions.clear();
        for(std::list<ThreatSolution*>::iterator pos = solutions.begin(); pos != solutions.end(); ++pos)
            delete *pos;
        solutions.clear();

        // Find all possible solutions on the board
        findClaimEvens();
        findBaseInverses();
        findVerticals();
        findLowInverses();
        findHighInverses();
        findBaseClaims();
        findBefores();
        findSpecialBefores();
        findAfterEvens();
        findAfterBaseInverses();    // It's best to call these 2 functions last, for if they find something it will solve all threats
        findAfterVerticals();       // If they're called last their solutions will be pushed in front of all other solutions and will therefore be tried first

        // Connect all solutions that can't be combined
        bool cantBeCombined = false;
        for(std::list<ThreatSolution*>::const_iterator pos = solutions.begin(); pos != solutions.end(); ++pos)
        {
            std::list<ThreatSolution*>::const_iterator pos2 = pos;
            ++pos2;
            for(; pos2 != solutions.end(); ++pos2)
            {
                // Get the restrictions to combine these two solutions
                const char restrictions = BoardExt::solutionCombinations[std::max((*pos)->type, (*pos2)->type)][std::min((*pos)->type, (*pos2)->type)];

                // Check whether the two solutions can be combined, using the restrictions
                cantBeCombined = false;
                if(restrictions & BoardExt::SC1)
                    cantBeCombined = !solutionCanCombine1(*pos, *pos2);
                if(!cantBeCombined && restrictions & BoardExt::SC2)
                    cantBeCombined = !solutionCanCombine2(*pos, *pos2);
                if(!cantBeCombined && restrictions & BoardExt::SC3)
                    cantBeCombined = !solutionCanCombine3(*pos, *pos2);
                if(!cantBeCombined && restrictions & BoardExt::SC4)
                    cantBeCombined = !solutionCanCombine4(*pos, *pos2);

                // If the solutions can't be combined, we connect them
                if(cantBeCombined)
                {
                    (*pos)->cantCombine.push_back(*pos2);
                    (*pos2)->cantCombine.push_back(*pos);
                }
            }
        }
    }

// Private:
    // Static:
        const char BoardExt::SC1 = 1;
        const char BoardExt::SC2 = 2;
        const char BoardExt::SC3 = 4;
        const char BoardExt::SC4 = 8;
        const char BoardExt::solutionCombinations[11][11] =
        {         /* CL     BI      VE      AE      LI      HI      BC      BE      SB      AB      AV */
        /* CL */    {SC1,   0,      0,      0,      0,      0,      0,      0,      0,      0,      0   },
        /* BI */    {SC1,   SC1,    0,      0,      0,      0,      0,      0,      0,      0,      0   },
        /* VE */    {SC1,   SC1,    SC1,    0,      0,      0,      0,      0,      0,      0,      0   },
        /* AE */    {SC1,   SC1,    SC1,    SC3,    0,      0,      0,      0,      0,      0,      0   },
        /* LI */    {SC2,   SC1,    SC1,    SC1|SC2,SC4,    0,      0,      0,      0,      0,      0   },
        /* HI */    {SC2,   SC1,    SC1,    SC1|SC2,SC4,    SC4,    0,      0,      0,      0,      0   },
        /* BC */    {SC1,   SC1,    SC1,    SC1,    SC1|SC2,SC1|SC2,SC1,    0,      0,      0,      0   },
        /* BE */    {SC1,   SC1,    SC1,    SC3,    SC2|SC3,SC1|SC2,SC1,    SC3,    0,      0,      0   },
        /* SB */    {SC1,   SC1,    SC1,    SC3,    SC2|SC3,SC1|SC2,SC1,    SC3,    SC3,    0,      0   },
        /* AB */    {SC1,   SC1,    SC1,    SC1,    SC1,    SC1,    SC1,    SC1,    SC1,    SC1,    0   },
        /* AV */    {SC1,   SC1,    SC1,    SC1,    SC1,    SC1,    SC1,    SC1,    SC1,    SC1,    SC1 }
        };

    bool BoardExt::findThreatsFromPoint(const int& col, const int& row, const LineThreat::Direction& dir)
    {
        const int deltaCol = dir == LineThreat::Vertical ? 0 : (dir == LineThreat::DiagonalLeft ? -1 : 1);
        const int deltaRow = dir == LineThreat::Horizontal ? 0 : -1;

        // If there isn't room for 4 pieces, there won't be a threat either
        if((col + 3 * deltaCol) < 0 || (col + 3 * deltaCol) >= 7  || (row + 3 * deltaRow) < 0) return false;

        // Check if a line of 4 pieces is possible and count how many pieces are already in place
        const Piece ownColor = isRed ? Red : Yellow;
        const Piece enemyColor = isRed ? Yellow : Red;
        int piecesInPlace = 0;
        for(int i = 0; i < 4; ++i)
        {
            const Piece currPiece = at(col + i * deltaCol)[row + i * deltaRow];

            // If a piece of our own color is found, the enemy can never get a line of 4 pieces
            // So no threat is found
            if(currPiece == ownColor)
                return false;

            // If an enemy piece is found we increase the amount of enemy pieces that's in place
            if(currPiece == enemyColor) ++piecesInPlace;
        }

        // Add the threat to the list
        LineThreat* pointer = new LineThreat(col, row, dir, piecesInPlace);
        threats.push_back(pointer);
        for(int i = 0; i < 4; ++i)
            threatBoard[col + i * deltaCol][row + i * deltaRow].push_back(pointer);
        return true;
    }
    bool BoardExt::findWinningThreatsFromPoint(const int& col, const int& row, const LineThreat::Direction& dir)
    {
        const int deltaCol = dir == LineThreat::Vertical ? 0 : (dir == LineThreat::DiagonalLeft ? -1 : 1);
        const int deltaRow = dir == LineThreat::Horizontal ? 0 : -1;

        // If there isn't room for 4 pieces, there won't be a threat either
        if((col + 3 * deltaCol) < 0 || (col + 3 * deltaCol) >= 7  || (row + 3 * deltaRow) < 0) return false;

        // Check if a line of 4 pieces is possible and count how many pieces are already in place
        const Piece ownColor = isRed ? Red : Yellow;
        const Piece enemyColor = isRed ? Yellow : Red;
        int piecesInPlace = 0;
        for(int i = 0; i < 4; ++i)
        {
            const Piece currPiece = at(col + i * deltaCol)[row + i * deltaRow];

            // If a piece of our own color is found, the enemy can never get a line of 4 pieces
            // So no threat is found
            if(currPiece == enemyColor)
                return false;

            // If an enemy piece is found we increase the amount of enemy pieces that's in place
            if(currPiece == ownColor) ++piecesInPlace;
        }

        // Add the threat to the list
        LineThreat* pointer = new LineThreat(col, row, dir, piecesInPlace);
        winningThreats.push_back(pointer);
        for(int i = 0; i < 4; ++i)
            winningThreatBoard[col + i * deltaCol][row + i * deltaRow].push_back(pointer);
        return true;
    }

    void BoardExt::findClaimEvens()
    {
        for(int col = 0; col < 7; ++col)
        {
            // Skip columns that are used by the odd threats
            if(col == oddThreatCol1 || col == oddThreatCol2) continue;

            for(int row = 5; row > 0; row -= 2)     // We start at 5 since that's in fact the sixth row (we start counting at 0)
            {
                if(at(col)[row] != Empty || at(col)[row - 1] != Empty) break;

                ThreatSolution* solution = new ThreatSolution(ThreatSolution::ClaimEven);
                solution->addSquare(col, row);
                solution->addSquare(col, row - 1);

                const std::list<LineThreat*>& solvedThreats = threatBoard[col][row];
                for(std::list<LineThreat*>::const_iterator pos = solvedThreats.begin(); pos != solvedThreats.end(); ++pos)
                {
                    solution->solvedThreats.push_back(*pos);
                    (*pos)->solutions.push_back(solution);
                }

                // A solution that solves nothing is of no use
                if(solution->solvedThreats.size() == 0)
                    delete solution;
                else
                    solutions.push_back(solution);
            }
        }
    }
    void BoardExt::findBaseInverses()
    {
        for(int col = 0; col < 7; ++col)
        {
            // Skip columns that are used by the odd threats
            if(col == oddThreatCol1 || col == oddThreatCol2) continue;

            // Skip non-playable columns
            if(playableCols[col] == -1) continue;

            for(int col2 = col + 1; col2 < col + 4; ++col2)
            {
                // Skip columns that are used by the odd threats
                if(col2 == oddThreatCol1 || col2 == oddThreatCol2) continue;

                // Skip non-playable columns
                if(playableCols[col2] == -1) continue;

                // Check if the two direct playable squares are part of the same group
                if(playableCols[col] == playableCols[col2] || col2 - col == std::abs(playableCols[col] - playableCols[col2]))
                {
                    ThreatSolution* solution = new ThreatSolution(ThreatSolution::BaseInverse);
                    solution->addSquare(col, playableCols[col]);
                    solution->addSquare(col2, playableCols[col2]);

                    const std::list<LineThreat*>& solvedThreats = threatBoard[col][playableCols[col]];
                    for(std::list<LineThreat*>::const_iterator pos = solvedThreats.begin(); pos != solvedThreats.end(); ++pos)
                    {
                        if((*pos)->coversCoords(col2, playableCols[col2]))
                        {
                            solution->solvedThreats.push_back(*pos);
                            (*pos)->solutions.push_back(solution);
                        }
                    }

                    // A solution that solves nothing is of no use
                    if(solution->solvedThreats.size() == 0)
                        delete solution;
                    else
                        solutions.push_back(solution);
                }
            }
        }
    }
    void BoardExt::findVerticals()
    {
        for(int col = 0; col < 7; ++col)
        {
            // Skip columns that are used by the odd threats
            if(col == oddThreatCol1 || col == oddThreatCol2) continue;

            for(int row = 4; row > 0; row -= 2)     // We start at 4 since that's in fact the fifth row (we start counting at 0)
            {
                if(at(col)[row] != Empty || at(col)[row - 1] != Empty) break;

                ThreatSolution* solution = new ThreatSolution(ThreatSolution::Vertical);
                solution->addSquare(col, row);
                solution->addSquare(col, row - 1);

                // If both squares have a level 3 threat we will eventually fill in one of them, winning the game for us
                // Also no threats above this Vertical will be filled in
                if(hasLevel3WinningThreat(col, row) && hasLevel3WinningThreat(col, row - 1))
                {
                    solution->makeGameWinner();
                    for(int row2 = row + 1; row2 < 6; ++row2)
                    {
                        const std::list<LineThreat*>& additionalSolves = threatBoard[col][row2];
                        for(std::list<LineThreat*>::const_iterator pos = additionalSolves.begin(); pos != additionalSolves.end(); ++pos)
                        {
                            solution->solvedThreats.push_back(*pos);
                            (*pos)->solutions.push_front(solution);
                        }
                    }
                }

                const std::list<LineThreat*>& solvedThreats = threatBoard[col][row - 1];
                for(std::list<LineThreat*>::const_iterator pos = solvedThreats.begin(); pos != solvedThreats.end(); ++pos)
                {
                    if((*pos)->dir == LineThreat::Vertical && (*pos)->startCoords().row != row - 1)
                    {
                        solution->solvedThreats.push_back(*pos);
                        if(solution->winsGame())
                            (*pos)->solutions.push_front(solution);
                        else
                            (*pos)->solutions.push_back(solution);
                    }
                }

                // A solution that solves nothing is of no use
                if(solution->solvedThreats.size() == 0)
                    delete solution;
                else if(solution->winsGame())
                    solutions.push_front(solution);
                else
                    solutions.push_back(solution);
            }
        }
    }
    void BoardExt::findAfterEvens()
    {
        PieceCoords coords;
        bool success;
        for(std::list<LineThreat*>::const_iterator pos = winningThreats.begin(); pos != winningThreats.end(); ++pos)
        {
            // Find out whether this solution can be used
            success = true;
            for(int i = 0; i < 4; ++i)
            {
                coords = (*pos)->at(i);
                if(at(coords.col)[coords.row] == Empty)
                {
                    // The even numbers are the odd rows since the numbers start at 0
                    // Also don't use columns that are used by the odd threats
                    if(coords.row % 2 == 0 || coords.col == oddThreatCol1 || coords.col == oddThreatCol2)
                    {
                        // We can't use a ClaimEven on this position, since it's an odd row
                        success = false;
                        break;
                    }
                    else if(at(coords.col)[coords.row - 1] != Empty)
                    {
                        // We can't use a ClaimEven on this position, since the lower square isn't empty
                        success = false;
                        break;
                    }
                }
            }

            // If it can be used we add it to the list
            if(success)
            {
                ThreatSolution* solution = new ThreatSolution(ThreatSolution::AfterEven);
                std::list<LineThreat*> solvedThreats;
                std::list<LineThreat*> possibleSolves;
                for(int i = 0; i < 4; ++i)
                {
                    coords = (*pos)->at(i);
                    if(at(coords.col)[coords.row] == Empty)
                    {
                        // Add the threats that are solved by the AfterEven
                        if(solution->squareCount() == 0)
                        {
                            // If this is the first AfterEven column, we add all threats in that column above the AfterEven group
                            for(int row = coords.row + 1; row < 6; ++row)
                            {
                                const std::list<LineThreat*>& solves = threatBoard[coords.col][row];
                                possibleSolves.insert(possibleSolves.end(), solves.begin(), solves.end());
                            }
                        }
                        else
                        {
                            // If this isn't the first AfterEven column, we check if the threats that we already added also lie in this column
                            // If they do, we keep them in the list. If they don't, we remove them from the list
                            bool coversCoords;
                            for(std::list<LineThreat*>::iterator pos2 = possibleSolves.begin(); pos2 != possibleSolves.end();)
                            {
                                coversCoords = false;
                                for(int row = coords.row + 1; row < 6; ++row)
                                {
                                    if((*pos2)->coversCoords(coords.col, row))
                                    {
                                        coversCoords = true;
                                        break;
                                    }
                                }
                                if(coversCoords)
                                    ++pos2;
                                else
                                    pos2 = possibleSolves.erase(pos2);
                            }
                        }

                        // Add the threats that are solved by the ClaimEvens used in this solution
                        const std::list<LineThreat*>& solvedByClaimEven = threatBoard[coords.col][coords.row];
                        solvedThreats.insert(solvedThreats.end(), solvedByClaimEven.begin(), solvedByClaimEven.end());

                        // Add the used squares to this solution
                        solution->addSquare(coords.col, coords.row);
                        solution->addSquare(coords.col, coords.row - 1);
                    }
                }

                // Add the threats that are solved by the ClaimEvens
                for(std::list<LineThreat*>::const_iterator pos2 = solvedThreats.begin(); pos2 != solvedThreats.end(); ++pos2)
                {
                    solution->solvedThreats.push_back(*pos2);
                    (*pos2)->solutions.push_front(solution);
                }
                // Add the threats that are solved by the AfterEven
                for(std::list<LineThreat*>::const_iterator pos2 = possibleSolves.begin(); pos2 != possibleSolves.end(); ++pos2)
                {
                    solution->solvedThreats.push_back(*pos2);
                    (*pos2)->solutions.push_front(solution);
                }

                // A solution that solves nothing is of no use
                if(solution->solvedThreats.size() == 0)
                    delete solution;
                else
                    solutions.push_front(solution);
            }
        }
    }
    void BoardExt::findLowInverses()
    {
        for(int col = 0; col < 7; ++col)
        {
            // Skip columns that are used by the odd threats
            if(col == oddThreatCol1 || col == oddThreatCol2) continue;

            // If the column isn't playable or the playable row is the last odd row (or above that one), there can't be a LowInverse
            if(playableCols[col] == -1 || playableCols[col] >= 4) continue;

            // Calculate from which row the low inverses start on this column
            const int fromRow = playableCols[col] + 1 - playableCols[col] % 2;

            for(int col2 = col + 1; col2 < 7; ++col2)
            {
                // Skip columns that are used by the odd threats
                if(col2 == oddThreatCol1 || col2 == oddThreatCol2) continue;

                // If the column isn't playable or the playable row is the last odd row (or above that one), there can't be a LowInverse
                if(playableCols[col2] == -1 || playableCols[col2] >= 4) continue;

                // Calculate from which row the low inverses start on this column
                const int fromRow2 = playableCols[col2] + 1 - playableCols[col2] % 2;

                // Check if the two direct playable squares are part of the same group
                for(int row = fromRow; row < 5; row += 2)   // row < 5, because the last row (number 5) shouldn't be included
                {
                    for(int row2 = fromRow2; row2 < 5; row2 += 2)
                    {
                        ThreatSolution* solution = new ThreatSolution(ThreatSolution::LowInverse);
                        solution->addSquare(col, row);
                        solution->addSquare(col, row + 1);
                        solution->addSquare(col2, row2);
                        solution->addSquare(col2, row2 + 1);

                        // If both upper squares have a level 3 threat we will eventually fill in one of them, winning the game for us
                        // Also no threats above both of these columns will be completed
                        if(hasLevel3WinningThreat(col, row + 1) && hasLevel3WinningThreat(col2, row2 + 1))
                        {
                            solution->makeGameWinner();
                            std::list<LineThreat*> additionalSolves;
                            for(int r = row + 2; r < 6; ++r)
                                additionalSolves.insert(additionalSolves.end(), threatBoard[col][r].begin(), threatBoard[col][r].end());
                            for(std::list<LineThreat*>::iterator pos = additionalSolves.begin(); pos != additionalSolves.end(); ++pos)
                            {
                                for(int r = row2 + 2; r < 6; ++r)
                                {
                                    if((*pos)->coversCoords(col2, r))
                                    {
                                        solution->solvedThreats.push_back(*pos);
                                        (*pos)->solutions.push_front(solution);
                                        break;
                                    }
                                }
                            }
                        }

                        // Add the threats that are solved by the Verticals
                        const std::list<LineThreat*>& solvedVerticals1 = threatBoard[col][row];
                        for(std::list<LineThreat*>::const_iterator pos = solvedVerticals1.begin(); pos != solvedVerticals1.end(); ++pos)
                        {
                            if((*pos)->dir == LineThreat::Vertical && (*pos)->startCoords().row != row)
                            {
                                solution->solvedThreats.push_back(*pos);
                                if(solution->winsGame())
                                    (*pos)->solutions.push_front(solution);
                                else
                                    (*pos)->solutions.push_back(solution);
                            }
                        }
                        const std::list<LineThreat*>& solvedVerticals2 = threatBoard[col2][row2];
                        for(std::list<LineThreat*>::const_iterator pos = solvedVerticals2.begin(); pos != solvedVerticals2.end(); ++pos)
                        {
                            if((*pos)->dir == LineThreat::Vertical && (*pos)->startCoords().row != row2)
                            {
                                solution->solvedThreats.push_back(*pos);
                                if(solution->winsGame())
                                    (*pos)->solutions.push_front(solution);
                                else
                                    (*pos)->solutions.push_back(solution);
                            }
                        }

                        // Add the threats that are solved by the LowInverse part
                        const std::list<LineThreat*>& solvedThreats = threatBoard[col][row + 1];
                        for(std::list<LineThreat*>::const_iterator pos = solvedThreats.begin(); pos != solvedThreats.end(); ++pos)
                        {
                            if((*pos)->coversCoords(col2, row2 + 1))
                            {
                                solution->solvedThreats.push_back(*pos);
                                if(solution->winsGame())
                                    (*pos)->solutions.push_front(solution);
                                else
                                    (*pos)->solutions.push_back(solution);
                            }
                        }

                        // A solution that solves nothing is of no use
                        if(solution->solvedThreats.size() == 0)
                            delete solution;
                        else if(solution->winsGame())
                            solutions.push_front(solution);
                        else
                            solutions.push_back(solution);
                    }
                }
            }
        }
    }
    void BoardExt::findHighInverses()
    {
        for(int col = 0; col < 7; ++col)
        {
            // Skip columns that are used by the odd threats
            if(col == oddThreatCol1 || col == oddThreatCol2) continue;

            // If the column isn't playable or the playable row is the last odd row (or above that one), there can't be a HighInverse
            if(playableCols[col] == -1 || playableCols[col] >= 4) continue;

            // Calculate from which row the high inverses start on this column
            const int fromRow = playableCols[col] + 1 - playableCols[col] % 2;

            for(int col2 = col + 1; col2 < 7; ++col2)
            {
                // Skip columns that are used by the odd threats
                if(col2 == oddThreatCol1 || col2 == oddThreatCol2) continue;

                // If the column isn't playable or the playable row is the last odd row (or above that one), there can't be a HighInverse
                if(playableCols[col2] == -1 || playableCols[col2] >= 4) continue;

                // Calculate from which row the high inverses start on this column
                const int fromRow2 = playableCols[col2] + 1 - playableCols[col2] % 2;

                // Check if the two direct playable squares are part of the same group
                for(int row = fromRow; row < 5; row += 2)           // row < 5, because the last row (number 5) shouldn't be included
                {
                    for(int row2 = fromRow2; row2 < 5; row2 += 2)
                    {
                        ThreatSolution* solution = new ThreatSolution(ThreatSolution::HighInverse);
                        solution->addSquare(col, row);
                        solution->addSquare(col, row + 1);
                        solution->addSquare(col, row + 2);
                        solution->addSquare(col2, row2);
                        solution->addSquare(col2, row2 + 1);
                        solution->addSquare(col2, row2 + 2);

                        // If both upper squares have a level 3 threat we will eventually fill in one of them, winning the game for us
                        // Also no threats above both of these columns will be completed
                        if(hasLevel3WinningThreat(col, row + 1) && hasLevel3WinningThreat(col2, row2 + 1))
                        {
                            solution->makeGameWinner();
                            std::list<LineThreat*> additionalSolves;
                            for(int r = row + 2; r < 6; ++r)
                                additionalSolves.insert(additionalSolves.end(), threatBoard[col][r].begin(), threatBoard[col][r].end());
                            for(std::list<LineThreat*>::iterator pos = additionalSolves.begin(); pos != additionalSolves.end(); ++pos)
                            {
                                for(int r = row2 + 2; r < 6; ++r)
                                {
                                    if((*pos)->coversCoords(col2, r))
                                    {
                                        solution->solvedThreats.push_back(*pos);
                                        (*pos)->solutions.push_front(solution);
                                        break;
                                    }
                                }
                            }
                        }
                        else if(playableCols[col] == row && hasLevel3WinningThreat(col, row) && hasLevel3WinningThreat(col2, row2 + 2))
                        {
                            solution->makeGameWinner();
                            std::list<LineThreat*> additionalSolves;
                            for(int r = row + 1; r < 6; ++r)
                                additionalSolves.insert(additionalSolves.end(), threatBoard[col][r].begin(), threatBoard[col][r].end());
                            for(std::list<LineThreat*>::iterator pos = additionalSolves.begin(); pos != additionalSolves.end(); ++pos)
                            {
                                for(int r = row2 + 3; r < 6; ++r)
                                {
                                    if((*pos)->coversCoords(col2, r))
                                    {
                                        solution->solvedThreats.push_back(*pos);
                                        (*pos)->solutions.push_front(solution);
                                        break;
                                    }
                                }
                            }
                        }
                        else if(playableCols[col2] == row2 && hasLevel3WinningThreat(col, row + 2) && hasLevel3WinningThreat(col2, row2))
                        {
                            solution->makeGameWinner();
                            std::list<LineThreat*> additionalSolves;
                            for(int r = row + 3; r < 6; ++r)
                                additionalSolves.insert(additionalSolves.end(), threatBoard[col][r].begin(), threatBoard[col][r].end());
                            for(std::list<LineThreat*>::iterator pos = additionalSolves.begin(); pos != additionalSolves.end(); ++pos)
                            {
                                for(int r = row2 + 1; r < 6; ++r)
                                {
                                    if((*pos)->coversCoords(col2, r))
                                    {
                                        solution->solvedThreats.push_back(*pos);
                                        (*pos)->solutions.push_front(solution);
                                        break;
                                    }
                                }
                            }
                        }
                        else if(hasLevel3WinningThreat(col, row + 2) && hasLevel3WinningThreat(col2, row2 + 2))
                        {
                            solution->makeGameWinner();
                            std::list<LineThreat*> additionalSolves;
                            for(int r = row + 3; r < 6; ++r)
                                additionalSolves.insert(additionalSolves.end(), threatBoard[col][r].begin(), threatBoard[col][r].end());
                            for(std::list<LineThreat*>::iterator pos = additionalSolves.begin(); pos != additionalSolves.end(); ++pos)
                            {
                                for(int r = row2 + 3; r < 6; ++r)
                                {
                                    if((*pos)->coversCoords(col2, r))
                                    {
                                        solution->solvedThreats.push_back(*pos);
                                        (*pos)->solutions.push_front(solution);
                                        break;
                                    }
                                }
                            }
                        }

                        // Add the threats that are solved by the Verticals
                        const std::list<LineThreat*>& solvedVerticals1 = threatBoard[col][row + 1];
                        for(std::list<LineThreat*>::const_iterator pos = solvedVerticals1.begin(); pos != solvedVerticals1.end(); ++pos)
                        {
                            if((*pos)->dir == LineThreat::Vertical && (*pos)->startCoords().row != row + 1)
                            {
                                solution->solvedThreats.push_back(*pos);
                                if(solution->winsGame())
                                    (*pos)->solutions.push_front(solution);
                                else
                                    (*pos)->solutions.push_back(solution);
                            }
                        }
                        const std::list<LineThreat*>& solvedVerticals2 = threatBoard[col2][row2 + 1];
                        for(std::list<LineThreat*>::const_iterator pos = solvedVerticals2.begin(); pos != solvedVerticals2.end(); ++pos)
                        {
                            if((*pos)->dir == LineThreat::Vertical && (*pos)->startCoords().row != row2 + 1)
                            {
                                solution->solvedThreats.push_back(*pos);
                                if(solution->winsGame())
                                    (*pos)->solutions.push_front(solution);
                                else
                                    (*pos)->solutions.push_back(solution);
                            }
                        }

                        // Add the threats that contain both middle squares
                        const std::list<LineThreat*>& solvedThreats1 = threatBoard[col][row + 1];
                        for(std::list<LineThreat*>::const_iterator pos = solvedThreats1.begin(); pos != solvedThreats1.end(); ++pos)
                        {
                            if((*pos)->coversCoords(col2, row2 + 1))
                            {
                                solution->solvedThreats.push_back(*pos);
                                if(solution->winsGame())
                                    (*pos)->solutions.push_front(solution);
                                else
                                    (*pos)->solutions.push_back(solution);
                            }
                        }
                        // Add the threats that contain both upper squares
                        const std::list<LineThreat*>& solvedThreats2 = threatBoard[col][row + 2];
                        for(std::list<LineThreat*>::const_iterator pos = solvedThreats2.begin(); pos != solvedThreats2.end(); ++pos)
                        {
                            if((*pos)->coversCoords(col2, row2 + 2))
                            {
                                solution->solvedThreats.push_back(*pos);
                                if(solution->winsGame())
                                    (*pos)->solutions.push_front(solution);
                                else
                                    (*pos)->solutions.push_back(solution);
                            }
                        }

                        // If the lower square of the first column is playable
                        // we add all threats that contain both the lower square of the first column and the upper square of the second column
                        if(playableCols[col] == row)
                        {
                            const std::list<LineThreat*>& solvedThreats = threatBoard[col][row];
                            for(std::list<LineThreat*>::const_iterator pos = solvedThreats.begin(); pos != solvedThreats.end(); ++pos)
                            {
                                if((*pos)->coversCoords(col2, row2 + 2))
                                {
                                    solution->solvedThreats.push_back(*pos);
                                    if(solution->winsGame())
                                        (*pos)->solutions.push_front(solution);
                                    else
                                        (*pos)->solutions.push_back(solution);
                                }
                            }
                        }

                        // If the lower square of the second column is playable
                        // we add all threats that contain both the lower square of the second column and the upper square of the first column
                        if(playableCols[col2] == row2)
                        {
                            const std::list<LineThreat*>& solvedThreats = threatBoard[col2][row2];
                            for(std::list<LineThreat*>::const_iterator pos = solvedThreats.begin(); pos != solvedThreats.end(); ++pos)
                            {
                                if((*pos)->coversCoords(col, row + 2))
                                {
                                    solution->solvedThreats.push_back(*pos);
                                    if(solution->winsGame())
                                        (*pos)->solutions.push_front(solution);
                                    else
                                        (*pos)->solutions.push_back(solution);
                                }
                            }
                        }

                        // A solution that solves nothing is of no use
                        if(solution->solvedThreats.size() == 0)
                            delete solution;
                        else if(solution->winsGame())
                            solutions.push_front(solution);
                        else
                            solutions.push_back(solution);
                    }
                }
            }
        }
    }
    void BoardExt::findBaseClaims()
    {
        for(int col = 0; col < 5; ++col)
        {
            // Skip columns that are used by the odd threats
            if(col == oddThreatCol1 || col == oddThreatCol2) continue;

            // Skip non-playable columns
            if(playableCols[col] == -1) continue;

            // Whether the square above the direct playable square in column 1 is even
            // This is true if the direct playable square is odd (which is true if its row number is even)
            const bool square1IsEven = playableCols[col] % 2 == 0;

            for(int col2 = col + 1; col2 < 6; ++col2)
            {
                // Skip columns that are used by the odd threats
                if(col2 == oddThreatCol1 || col2 == oddThreatCol2) continue;

                // Skip non-playable columns
                if(playableCols[col2] == -1) continue;

                // Whether the square above the direct playable square in column 2 is even
                // This is true if the direct playable square is odd (which is true if its row number is even)
                const bool square2IsEven = playableCols[col2] % 2 == 0;

                for(int col3 = col2 + 1; col3 < 7; ++col3)
                {
                    // Skip columns that are used by the odd threats
                    if(col3 == oddThreatCol1 || col3 == oddThreatCol2) continue;

                    // Skip non-playable columns
                    if(playableCols[col3] == -1) continue;

                    // Whether the square above the direct playable square in column 3 is even
                    // This is true if the direct playable square is odd (which is true if its row number is even)
                    const bool square3IsEven = playableCols[col3] % 2 == 0;

                    // A BaseClaim is possible if the square above the direct playable square in column 1 is even
                    if(square1IsEven)
                    {
                        // Find which threats are solved by the BaseInverse
                        std::list<LineThreat*> solvedByBaseInverse;
                        const std::list<LineThreat*>& possibleBaseInverseSolves = threatBoard[col2][playableCols[col2]];
                        for(std::list<LineThreat*>::const_iterator pos = possibleBaseInverseSolves.begin(); pos != possibleBaseInverseSolves.end(); ++pos)
                        {
                            if((*pos)->coversCoords(col3, playableCols[col3]))
                                solvedByBaseInverse.push_back(*pos);
                        }

                        // First variant of the solution
                        ThreatSolution* solution1 = new ThreatSolution(ThreatSolution::BaseClaim);
                        solution1->addSquare(col, playableCols[col]);
                        solution1->addSquare(col2, playableCols[col2]);
                        solution1->addSquare(col3, playableCols[col3]);
                        solution1->addSquare(col, playableCols[col] + 1);

                        // Check if this solution wins the game
                        if(hasLevel3WinningThreat(col2, playableCols[col2]) && hasLevel3WinningThreat(col, playableCols[col] + 1))
                        {
                            solution1->makeGameWinner();
                            std::list<LineThreat*> additionalSolves;
                            for(int r = playableCols[col] + 2; r < 6; ++r)
                                additionalSolves.insert(additionalSolves.end(), threatBoard[col][r].begin(), threatBoard[col][r].end());
                            for(std::list<LineThreat*>::iterator pos = additionalSolves.begin(); pos != additionalSolves.end(); ++pos)
                            {
                                for(int r = playableCols[col2] + 1; r < 6; ++r)
                                {
                                    if((*pos)->coversCoords(col2, r))
                                    {
                                        solution1->solvedThreats.push_back(*pos);
                                        (*pos)->solutions.push_front(solution1);
                                        break;
                                    }
                                }
                            }
                        }
                        // Checking for a win on the second and third playable sqaure is not necessary
                        // since that would be an AfterBaseInverse which solves all threats and wins the game

                        const std::list<LineThreat*>& solvedThreats1 = threatBoard[col2][playableCols[col2]];
                        for(std::list<LineThreat*>::const_iterator pos = solvedThreats1.begin(); pos != solvedThreats1.end(); ++pos)
                        {
                            if((*pos)->coversCoords(col, playableCols[col] + 1))
                            {
                                solution1->solvedThreats.push_back(*pos);
                                if(solution1->winsGame())
                                    (*pos)->solutions.push_front(solution1);
                                else
                                    (*pos)->solutions.push_back(solution1);
                            }
                        }
                        for(std::list<LineThreat*>::const_iterator pos = solvedByBaseInverse.begin(); pos != solvedByBaseInverse.end(); ++pos)
                        {
                            solution1->solvedThreats.push_back(*pos);
                            if(solution1->winsGame())
                                (*pos)->solutions.push_front(solution1);
                            else
                                (*pos)->solutions.push_back(solution1);
                        }

                        // A solution that solves nothing is of no use
                        if(solution1->solvedThreats.size() == 0)
                            delete solution1;
                        else if(solution1->winsGame())
                            solutions.push_front(solution1);
                        else
                            solutions.push_back(solution1);

                        // Second variant of the solution
                        ThreatSolution* solution2 = new ThreatSolution(ThreatSolution::BaseClaim);
                        solution2->addSquare(col, playableCols[col]);
                        solution2->addSquare(col2, playableCols[col2]);
                        solution2->addSquare(col3, playableCols[col3]);
                        solution2->addSquare(col, playableCols[col] + 1);

                        // Check if this solution wins the game
                        if(hasLevel3WinningThreat(col3, playableCols[col3]) && hasLevel3WinningThreat(col, playableCols[col] + 1))
                        {
                            solution2->makeGameWinner();
                            std::list<LineThreat*> additionalSolves;
                            for(int r = playableCols[col] + 2; r < 6; ++r)
                                additionalSolves.insert(additionalSolves.end(), threatBoard[col][r].begin(), threatBoard[col][r].end());
                            for(std::list<LineThreat*>::iterator pos = additionalSolves.begin(); pos != additionalSolves.end(); ++pos)
                            {
                                for(int r = playableCols[col3] + 1; r < 6; ++r)
                                {
                                    if((*pos)->coversCoords(col3, r))
                                    {
                                        solution2->solvedThreats.push_back(*pos);
                                        (*pos)->solutions.push_front(solution2);
                                        break;
                                    }
                                }
                            }
                        }
                        // Checking for a win on the second and third playable sqaure is not necessary
                        // since that would be an AfterBaseInverse which solves all threats and wins the game

                        const std::list<LineThreat*>& solvedThreats2 = threatBoard[col3][playableCols[col3]];
                        for(std::list<LineThreat*>::const_iterator pos = solvedThreats2.begin(); pos != solvedThreats2.end(); ++pos)
                        {
                            if((*pos)->coversCoords(col, playableCols[col] + 1))
                            {
                                solution2->solvedThreats.push_back(*pos);
                                if(solution2->winsGame())
                                    (*pos)->solutions.push_front(solution2);
                                else
                                    (*pos)->solutions.push_back(solution2);
                            }
                        }
                        for(std::list<LineThreat*>::const_iterator pos = solvedByBaseInverse.begin(); pos != solvedByBaseInverse.end(); ++pos)
                        {
                            solution2->solvedThreats.push_back(*pos);
                            if(solution2->winsGame())
                                (*pos)->solutions.push_front(solution2);
                            else
                                (*pos)->solutions.push_back(solution2);
                        }

                        // A solution that solves nothing is of no use
                        if(solution2->solvedThreats.size() == 0)
                            delete solution2;
                        else if(solution2->winsGame())
                            solutions.push_front(solution2);
                        else
                            solutions.push_back(solution2);
                    }
                    // It's also possible if the square above the direct playable square in column 2 is even
                    if(square2IsEven)
                    {
                        // Find which threats are solved by the BaseInverse
                        std::list<LineThreat*> solvedByBaseInverse;
                        const std::list<LineThreat*>& possibleBaseInverseSolves = threatBoard[col][playableCols[col]];
                        for(std::list<LineThreat*>::const_iterator pos = possibleBaseInverseSolves.begin(); pos != possibleBaseInverseSolves.end(); ++pos)
                        {
                            if((*pos)->coversCoords(col3, playableCols[col3]))
                                solvedByBaseInverse.push_back(*pos);
                        }

                        // First variant of the solution
                        ThreatSolution* solution1 = new ThreatSolution(ThreatSolution::BaseClaim);
                        solution1->addSquare(col, playableCols[col]);
                        solution1->addSquare(col2, playableCols[col2]);
                        solution1->addSquare(col3, playableCols[col3]);
                        solution1->addSquare(col2, playableCols[col2] + 1);

                        // Check if this solution wins the game
                        if(hasLevel3WinningThreat(col, playableCols[col]) && hasLevel3WinningThreat(col2, playableCols[col2] + 1))
                        {
                            solution1->makeGameWinner();
                            std::list<LineThreat*> additionalSolves;
                            for(int r = playableCols[col2] + 2; r < 6; ++r)
                                additionalSolves.insert(additionalSolves.end(), threatBoard[col2][r].begin(), threatBoard[col2][r].end());
                            for(std::list<LineThreat*>::iterator pos = additionalSolves.begin(); pos != additionalSolves.end(); ++pos)
                            {
                                for(int r = playableCols[col] + 1; r < 6; ++r)
                                {
                                    if((*pos)->coversCoords(col, r))
                                    {
                                        solution1->solvedThreats.push_back(*pos);
                                        (*pos)->solutions.push_front(solution1);
                                        break;
                                    }
                                }
                            }
                        }
                        // Checking for a win on the second and third playable sqaure is not necessary
                        // since that would be an AfterBaseInverse which solves all threats and wins the game

                        const std::list<LineThreat*>& solvedThreats1 = threatBoard[col][playableCols[col]];
                        for(std::list<LineThreat*>::const_iterator pos = solvedThreats1.begin(); pos != solvedThreats1.end(); ++pos)
                        {
                            if((*pos)->coversCoords(col2, playableCols[col2] + 1))
                            {
                                solution1->solvedThreats.push_back(*pos);
                                if(solution1->winsGame())
                                    (*pos)->solutions.push_front(solution1);
                                else
                                    (*pos)->solutions.push_back(solution1);
                            }
                        }
                        for(std::list<LineThreat*>::const_iterator pos = solvedByBaseInverse.begin(); pos != solvedByBaseInverse.end(); ++pos)
                        {
                            solution1->solvedThreats.push_back(*pos);
                            if(solution1->winsGame())
                                (*pos)->solutions.push_front(solution1);
                            else
                                (*pos)->solutions.push_back(solution1);
                        }

                        // A solution that solves nothing is of no use
                        if(solution1->solvedThreats.size() == 0)
                            delete solution1;
                        else if(solution1->winsGame())
                            solutions.push_front(solution1);
                        else
                            solutions.push_back(solution1);

                        // Second variant of the solution
                        ThreatSolution* solution2 = new ThreatSolution(ThreatSolution::BaseClaim);
                        solution2->addSquare(col, playableCols[col]);
                        solution2->addSquare(col2, playableCols[col2]);
                        solution2->addSquare(col3, playableCols[col3]);
                        solution2->addSquare(col2, playableCols[col2] + 1);

                        // Check if this solution wins the game
                        if(hasLevel3WinningThreat(col3, playableCols[col3]) && hasLevel3WinningThreat(col2, playableCols[col2] + 1))
                        {
                            solution2->makeGameWinner();
                            std::list<LineThreat*> additionalSolves;
                            for(int r = playableCols[col2] + 2; r < 6; ++r)
                                additionalSolves.insert(additionalSolves.end(), threatBoard[col2][r].begin(), threatBoard[col2][r].end());
                            for(std::list<LineThreat*>::iterator pos = additionalSolves.begin(); pos != additionalSolves.end(); ++pos)
                            {
                                for(int r = playableCols[col3] + 1; r < 6; ++r)
                                {
                                    if((*pos)->coversCoords(col3, r))
                                    {
                                        solution2->solvedThreats.push_back(*pos);
                                        (*pos)->solutions.push_front(solution2);
                                        break;
                                    }
                                }
                            }
                        }
                        // Checking for a win on the second and third playable sqaure is not necessary
                        // since that would be an AfterBaseInverse which solves all threats and wins the game

                        const std::list<LineThreat*>& solvedThreats2 = threatBoard[col3][playableCols[col3]];
                        for(std::list<LineThreat*>::const_iterator pos = solvedThreats2.begin(); pos != solvedThreats2.end(); ++pos)
                        {
                            if((*pos)->coversCoords(col2, playableCols[col2] + 1))
                            {
                                solution2->solvedThreats.push_back(*pos);
                                if(solution2->winsGame())
                                    (*pos)->solutions.push_front(solution2);
                                else
                                    (*pos)->solutions.push_back(solution2);
                            }
                        }
                        for(std::list<LineThreat*>::const_iterator pos = solvedByBaseInverse.begin(); pos != solvedByBaseInverse.end(); ++pos)
                        {
                            solution2->solvedThreats.push_back(*pos);
                            if(solution2->winsGame())
                                (*pos)->solutions.push_front(solution2);
                            else
                                (*pos)->solutions.push_back(solution2);
                        }

                        // A solution that solves nothing is of no use
                        if(solution2->solvedThreats.size() == 0)
                            delete solution2;
                        else if(solution2->winsGame())
                            solutions.push_front(solution2);
                        else
                            solutions.push_back(solution2);
                    }
                    // And last, but not least: when the square above the direct playable square in column 3 is even
                    if(square3IsEven)
                    {
                        // Find which threats are solved by the BaseInverse
                        std::list<LineThreat*> solvedByBaseInverse;
                        const std::list<LineThreat*>& possibleBaseInverseSolves = threatBoard[col2][playableCols[col2]];
                        for(std::list<LineThreat*>::const_iterator pos = possibleBaseInverseSolves.begin(); pos != possibleBaseInverseSolves.end(); ++pos)
                        {
                            if((*pos)->coversCoords(col, playableCols[col]))
                                solvedByBaseInverse.push_back(*pos);
                        }

                        // First variant of the solution
                        ThreatSolution* solution1 = new ThreatSolution(ThreatSolution::BaseClaim);
                        solution1->addSquare(col, playableCols[col]);
                        solution1->addSquare(col2, playableCols[col2]);
                        solution1->addSquare(col3, playableCols[col3]);
                        solution1->addSquare(col3, playableCols[col3] + 1);

                        // Check if this solution wins the game
                        if(hasLevel3WinningThreat(col, playableCols[col]) && hasLevel3WinningThreat(col3, playableCols[col3] + 1))
                        {
                            solution1->makeGameWinner();
                            std::list<LineThreat*> additionalSolves;
                            for(int r = playableCols[col3] + 2; r < 6; ++r)
                                additionalSolves.insert(additionalSolves.end(), threatBoard[col3][r].begin(), threatBoard[col3][r].end());
                            for(std::list<LineThreat*>::iterator pos = additionalSolves.begin(); pos != additionalSolves.end(); ++pos)
                            {
                                for(int r = playableCols[col] + 1; r < 6; ++r)
                                {
                                    if((*pos)->coversCoords(col, r))
                                    {
                                        solution1->solvedThreats.push_back(*pos);
                                        (*pos)->solutions.push_front(solution1);
                                        break;
                                    }
                                }
                            }
                        }
                        // Checking for a win on the second and third playable sqaure is not necessary
                        // since that would be an AfterBaseInverse which solves all threats and wins the game

                        const std::list<LineThreat*>& solvedThreats1 = threatBoard[col][playableCols[col]];
                        for(std::list<LineThreat*>::const_iterator pos = solvedThreats1.begin(); pos != solvedThreats1.end(); ++pos)
                        {
                            if((*pos)->coversCoords(col3, playableCols[col3] + 1))
                            {
                                solution1->solvedThreats.push_back(*pos);
                                if(solution1->winsGame())
                                    (*pos)->solutions.push_front(solution1);
                                else
                                    (*pos)->solutions.push_back(solution1);
                            }
                        }
                        for(std::list<LineThreat*>::const_iterator pos = solvedByBaseInverse.begin(); pos != solvedByBaseInverse.end(); ++pos)
                        {
                            solution1->solvedThreats.push_back(*pos);
                            if(solution1->winsGame())
                                (*pos)->solutions.push_front(solution1);
                            else
                                (*pos)->solutions.push_back(solution1);
                        }

                        // A solution that solves nothing is of no use
                        if(solution1->solvedThreats.size() == 0)
                            delete solution1;
                        else if(solution1->winsGame())
                            solutions.push_front(solution1);
                        else
                            solutions.push_back(solution1);

                        // Second variant of the solution
                        ThreatSolution* solution2 = new ThreatSolution(ThreatSolution::BaseClaim);
                        solution2->addSquare(col, playableCols[col]);
                        solution2->addSquare(col2, playableCols[col2]);
                        solution2->addSquare(col3, playableCols[col3]);
                        solution2->addSquare(col3, playableCols[col3] + 1);

                        // Check if this solution wins the game
                        if(hasLevel3WinningThreat(col2, playableCols[col2]) && hasLevel3WinningThreat(col3, playableCols[col3] + 1))
                        {
                            solution2->makeGameWinner();
                            std::list<LineThreat*> additionalSolves;
                            for(int r = playableCols[col3] + 2; r < 6; ++r)
                                additionalSolves.insert(additionalSolves.end(), threatBoard[col3][r].begin(), threatBoard[col3][r].end());
                            for(std::list<LineThreat*>::iterator pos = additionalSolves.begin(); pos != additionalSolves.end(); ++pos)
                            {
                                for(int r = playableCols[col2] + 1; r < 6; ++r)
                                {
                                    if((*pos)->coversCoords(col2, r))
                                    {
                                        solution2->solvedThreats.push_back(*pos);
                                        (*pos)->solutions.push_front(solution2);
                                        break;
                                    }
                                }
                            }
                        }
                        // Checking for a win on the second and third playable sqaure is not necessary
                        // since that would be an AfterBaseInverse which solves all threats and wins the game

                        const std::list<LineThreat*>& solvedThreats2 = threatBoard[col2][playableCols[col2]];
                        for(std::list<LineThreat*>::const_iterator pos = solvedThreats2.begin(); pos != solvedThreats2.end(); ++pos)
                        {
                            if((*pos)->coversCoords(col3, playableCols[col3] + 1))
                            {
                                solution2->solvedThreats.push_back(*pos);
                                if(solution2->winsGame())
                                    (*pos)->solutions.push_front(solution2);
                                else
                                    (*pos)->solutions.push_back(solution2);
                            }
                        }
                        for(std::list<LineThreat*>::const_iterator pos = solvedByBaseInverse.begin(); pos != solvedByBaseInverse.end(); ++pos)
                        {
                            solution2->solvedThreats.push_back(*pos);
                            if(solution2->winsGame())
                                (*pos)->solutions.push_front(solution2);
                            else
                                (*pos)->solutions.push_back(solution2);
                        }

                        // A solution that solves nothing is of no use
                        if(solution2->solvedThreats.size() == 0)
                            delete solution2;
                        else if(solution2->winsGame())
                            solutions.push_front(solution2);
                        else
                            solutions.push_back(solution2);
                    }
                }
            }
        }
    }
    void BoardExt::findBefores()
    {
        PieceCoords coords;
        bool success;
        for(std::list<LineThreat*>::const_iterator pos = winningThreats.begin(); pos != winningThreats.end(); ++pos)
        {
            // Find out whether this solution can be used
            success = true;
            for(int i = 0; i < 4; ++i)
            {
                coords = (*pos)->at(i);
                // None of the empty squares should lie in the upper row of the board
                // nor should a column be used by the odd threats
                if((at(coords.col)[coords.row] == Empty && coords.row == 5) || coords.col == oddThreatCol1 || coords.col == oddThreatCol2)
                {
                    success = false;
                    break;
                }
            }

            // If it can be used we add it to the list
            if(success)
            {
                bool firstSquare = true;
                std::list<LineThreat*> possibleSolves;
                std::list<LineThreat*> solvedVertical[4]    = {std::list<LineThreat*>(), std::list<LineThreat*>(), std::list<LineThreat*>(), std::list<LineThreat*>()};
                std::list<LineThreat*> solvedClaimEven[4]   = {std::list<LineThreat*>(), std::list<LineThreat*>(), std::list<LineThreat*>(), std::list<LineThreat*>()};

                for(int i = 0; i < 4; ++i)
                {
                    coords = (*pos)->at(i);
                    if(at(coords.col)[coords.row] == Empty)
                    {
                        // Add the threats that are solved by the Before
                        if(firstSquare)
                        {
                            // If this is the first empy Before square, we add all threats on that square
                            possibleSolves = threatBoard[coords.col][coords.row + 1];
                            firstSquare = false;
                        }
                        else
                        {
                            // If this isn't the first empty Before square, we check if the threats that we already added also contain this square
                            // If they do, we keep them in the list. If they don't, we remove them from the list
                            for(std::list<LineThreat*>::iterator pos2 = possibleSolves.begin(); pos2 != possibleSolves.end();)
                            {
                                if((*pos2)->coversCoords(coords.col, coords.row + 1))
                                    ++pos2;
                                else
                                    pos2 = possibleSolves.erase(pos2);
                            }
                        }

                        // A ClaimEven on this square and the square below it can only be used if the square below is is empty and if this square is even
                        // Also, if the Before group is a Vertical, only a ClaimEven can be used on the lower square
                        if(at(coords.col)[coords.row - 1] == Empty && coords.row % 2 != 0 && ((*pos)->dir != LineThreat::Vertical || coords.row == (*pos)->startCoords().row))
                            solvedClaimEven[i] = threatBoard[coords.col][coords.row];

                        // Add all threats that are solved by a Vertical with its lowest square in the Before group
                        const std::list<LineThreat*>& solvedByVertical = threatBoard[coords.col][coords.row];
                        for(std::list<LineThreat*>::const_iterator pos = solvedByVertical.begin(); pos != solvedByVertical.end(); ++pos)
                        {
                            if((*pos)->dir == LineThreat::Vertical && (*pos)->startCoords().row != coords.row)
                                solvedVertical[i].push_back(*pos);
                        }
                    }
                }

                // Create all 16 possible solutions
                // For each of the 4 squares there are 2 possibilities:
                //  - A Vertical using the square in the Before group and the square above it
                //  - A ClaimEven on the square in the Before group
                // Therefore the number of possibilities is 2^4 = 16
                // Each possibility can be represented by a binary number where a 1 indicates the use of a ClaimEven
                // We loopt to 14, not 15 because that one would consist of only ClaimEvens which would make the Before an AfterEven
                for(int i = 0; i < 15; ++i)
                {
                    // Create the solution
                    ThreatSolutionBefore* solution = new ThreatSolutionBefore(ThreatSolution::Before);

                    // Check if the solution is possible
                    // That is, when we're not trying to use a square below the Before group that's not empty
                    // and when we're not trying to use a ClaimEven on an uneven square
                    // We also add the used squares in this loop
                    bool illegalSolution = false;
                    bool isAfterEven = true;
                    for(int j = 0; j < 4; ++j)
                    {
                        coords = (*pos)->at(j);
                        if(at(coords.col)[coords.row] == Empty)
                        {
                            // The square in the Before group and the one above it is always used
                            solution->addSquare(coords.col, coords.row);

                            // Whether we should use the ClaimEven version or not
                            if(i & (1 << j))
                            {
                                if(coords.row % 2 == 0 || at(coords.col)[coords.row - 1] != Empty || ((*pos)->dir == LineThreat::Vertical && coords.row != (*pos)->startCoords().row))
                                {
                                    illegalSolution = true;
                                    break;
                                }
                                else
                                {
                                    solution->useClaimEvenIn(coords.col);
                                    solution->addSquare(coords.col, coords.row - 1);
                                }
                            }
                            else
                            {
                                solution->addSquare(coords.col, coords.row + 1);
                                isAfterEven = false;
                            }
                        }
                    }
                    if(illegalSolution || isAfterEven)
                    {
                        delete solution;
                        continue;
                    }

                    // Add the threats that are solved by Before
                    for(std::list<LineThreat*>::const_iterator pos2 = possibleSolves.begin(); pos2 != possibleSolves.end(); ++pos2)
                    {
                        solution->solvedThreats.push_back(*pos2);
                        (*pos2)->solutions.push_back(solution);
                    }

                    // Add the threats that are solved by the ClaimEvens and Verticals
                    for(int j = 0; j < 4; ++j)
                    {
                        // Get the right list of threats to add
                        const std::list<LineThreat*>& currList = i & (1 << j) ? solvedClaimEven[j] : solvedVertical[j];

                        // Add the threats
                        for(std::list<LineThreat*>::const_iterator pos2 = currList.begin(); pos2 != currList.end(); ++pos2)
                        {
                            solution->solvedThreats.push_back(*pos2);
                            (*pos2)->solutions.push_back(solution);
                        }
                    }

                    // A solution that solves nothing is of no use
                    if(solution->solvedThreats.size() == 0)
                        delete solution;
                    else
                        solutions.push_back(solution);
                }
            }
        }
    }
    void BoardExt::findSpecialBefores()
    {
        for(int col = 0; col < 7; ++col)
        {
            // Skip columns that are used by the odd threats
            if(col == oddThreatCol1 || col == oddThreatCol2) continue;

            // Skip non-playable columns
            const int& row = playableCols[col];
            if(row == -1) continue;

            const std::list<LineThreat*>& winThreats = winningThreatBoard[col][row];
            for(std::list<LineThreat*>::const_iterator pos = winThreats.begin(); pos != winThreats.end(); ++pos)
            {
                // Check if it's a valid group
                bool validGroup = true;
                for(int i = 0; i < 4; ++i)
                {
                    // No empty square may lay in the upper row of the board
                    // nor should a column be used by the odd threats
                    if((at((*pos)->at(i).col)[(*pos)->at(i).row] == Empty && (*pos)->at(i).row == 5) || (*pos)->at(i).col == oddThreatCol1 || (*pos)->at(i).col == oddThreatCol2)
                    {
                        validGroup = false;
                        break;
                    }
                }
                if(!validGroup) continue;

                for(int col2 = 0; col2 < 7; ++col2)
                {
                    // Skip the columns of the SpecialBefore group
                    if(col2 == (*pos)->mostLeftCol())
                    {
                        col2 = (*pos)->mostRightCol();
                        continue;
                    }

                    // If no square is playable in this column, we skip it
                    const int& row2 = playableCols[col2];
                    if(row2 == -1) continue;

                    // Find the threats that are solved by this SpecialBefore
                    PieceCoords coords;
                    std::list<LineThreat*> possibleSolves = threatBoard[col2][row2];
                    std::list<LineThreat*> solvedByDirectPlayable;
                    std::list<LineThreat*> solvedVertical[4]    = {std::list<LineThreat*>(), std::list<LineThreat*>(), std::list<LineThreat*>(), std::list<LineThreat*>()};
                    std::list<LineThreat*> solvedClaimEven[4]   = {std::list<LineThreat*>(), std::list<LineThreat*>(), std::list<LineThreat*>(), std::list<LineThreat*>()};

                    for(std::list<LineThreat*>::const_iterator pos2 = possibleSolves.begin(); pos2 != possibleSolves.end(); ++pos2)
                    {
                        if((*pos2)->coversCoords(col, row))
                            solvedByDirectPlayable.push_back(*pos2);
                    }
                    for(int i = 0; i < 4; ++i)
                    {
                        coords = (*pos)->at(i);
                        if(at(coords.col)[coords.row] == Empty)
                        {
                            // If this isn't the first empty SpecialBefore square, we check if the threats that we already added also contain this square
                            // If they do, we keep them in the list. If they don't, we remove them from the list
                            for(std::list<LineThreat*>::iterator pos2 = possibleSolves.begin(); pos2 != possibleSolves.end();)
                            {
                                if((*pos2)->coversCoords(coords.col, coords.row + 1))
                                    ++pos2;
                                else
                                    pos2 = possibleSolves.erase(pos2);
                            }

                            // A ClaimEven on this square and the square below it can only be used if the square below is is empty and if this square is even
                            // Also, the type of the SpecialBefore group may not be a Vertical, since a ClaimEven can only be used on the bottom square of the group
                            // and since one playable square in the SpecialBefore group is needed (which will always be the bottom square) a ClaimEven is not possible
                            if(at(coords.col)[coords.row - 1] == Empty && coords.row % 2 != 0 && (*pos)->dir != LineThreat::Vertical)
                                solvedClaimEven[i] = threatBoard[coords.col][coords.row];

                            // Add all threats that are solved by a Vertical with its lowest square in the SpecialBefore group
                            const std::list<LineThreat*>& solvedByVertical = threatBoard[coords.col][coords.row];
                            for(std::list<LineThreat*>::const_iterator pos = solvedByVertical.begin(); pos != solvedByVertical.end(); ++pos)
                            {
                                if((*pos)->dir == LineThreat::Vertical && (*pos)->startCoords().row != coords.row)
                                    solvedVertical[i].push_back(*pos);
                            }
                        }
                    }

                    // Create all 16 possible solutions
                    // For each of the 4 squares there are 2 possibilities:
                    //  - A Vertical using the square in the SpecialBefore group and the square above it
                    //  - A ClaimEven on the square in the SpecialBefore group
                    // Therefore the number of possibilities is 2^4 = 16
                    // Each possibility can be represented by a binary number where a 1 indicates the use of a ClaimEven
                    for(int i = 0; i < 16; ++i)
                    {
                        // Create the solution
                        ThreatSolutionBefore* solution = new ThreatSolutionBefore(ThreatSolution::SpecialBefore);
                        solution->addSquare(col2, row2);
                        solution->addSpecialSquareColumn(col);
                        solution->addSpecialSquareColumn(col2);

                        // Check if the solution is possible
                        // That is, when we're not trying to use a square below the SpecialBefore group that's not empty
                        // and when we're not trying to use a ClaimEven on an uneven square
                        // We also add the used squares in this loop
                        bool illegalSolution = false;
                        for(int j = 0; j < 4; ++j)
                        {
                            coords = (*pos)->at(j);
                            if(at(coords.col)[coords.row] == Empty)
                            {
                                // The square in the Before group and the one above it is always used
                                solution->addSquare(coords.col, coords.row);

                                // Whether we should use the ClaimEven version or not
                                if(i & (1 << j))
                                {
                                    if(coords.row % 2 == 0 || at(coords.col)[coords.row - 1] != Empty || (*pos)->dir == LineThreat::Vertical)
                                    {
                                        illegalSolution = true;
                                        break;
                                    }
                                    else
                                    {
                                        solution->useClaimEvenIn(coords.col);
                                        solution->addSquare(coords.col, coords.row - 1);
                                    }
                                }
                                else
                                    solution->addSquare(coords.col, coords.row + 1);
                            }
                        }
                        if(illegalSolution)
                        {
                            delete solution;
                            continue;
                        }

                        // Add the threats that are solved by the directly playable squares
                        for(std::list<LineThreat*>::const_iterator pos2 = solvedByDirectPlayable.begin(); pos2 != solvedByDirectPlayable.end(); ++pos2)
                        {
                            solution->solvedThreats.push_back(*pos2);
                            (*pos2)->solutions.push_back(solution);
                        }

                        // Add the threats that are solved by SpecialBefore
                        for(std::list<LineThreat*>::const_iterator pos2 = possibleSolves.begin(); pos2 != possibleSolves.end(); ++pos2)
                        {
                            solution->solvedThreats.push_back(*pos2);
                            (*pos2)->solutions.push_back(solution);
                        }

                        // Add the threats that are solved by the ClaimEvens and Verticals
                        for(int j = 0; j < 4; ++j)
                        {
                            // Get the right list of threats to add
                            const std::list<LineThreat*>& currList = i & (1 << j) ? solvedClaimEven[j] : solvedVertical[j];

                            // Add the threats
                            for(std::list<LineThreat*>::const_iterator pos2 = currList.begin(); pos2 != currList.end(); ++pos2)
                            {
                                solution->solvedThreats.push_back(*pos2);
                                (*pos2)->solutions.push_back(solution);
                            }
                        }

                        // A solution that solves nothing is of no use
                        if(solution->solvedThreats.size() == 0)
                            delete solution;
                        else
                            solutions.push_back(solution);
                    }
                }
            }
        }
    }

    void BoardExt::findAfterBaseInverses()
    {
        // Note that this solution doesn't need to skip the columns that are used by the odd threat,
        // since we will win the game the next turn anyway when using this solution

        // Check how many direct playable threats (of level 3) we have
        int directThreats = 0;
        ThreatSolution* solution = new ThreatSolution(ThreatSolution::AfterBaseInverse);
        for(int col = 0; col < 7; ++col)
        {
            if(playableCols[col] == -1) continue;

            if(hasLevel3WinningThreat(col, playableCols[col]))
            {
                ++directThreats;
                solution->addSquare(col, playableCols[col]);
            }

            if(directThreats == 2) break;
        }

        // If we've got a direct completable threat in two different columns we will win in our next move
        // The opponent can only fill in one threat and we can win on the other threat
        // Since there are no direct threats of the opponent (this function wouldn't be called if there are) we solve all threats
        if(directThreats == 2)
        {
            solution->solvedThreats = std::vector<LineThreat*>(threats.begin(), threats.end());
            for(std::list<LineThreat*>::const_iterator pos = threats.begin(); pos != threats.end(); ++pos)
                (*pos)->solutions.push_back(solution);
            solutions.push_front(solution);
        }
        else
            delete solution;
    }

    void BoardExt::findAfterVerticals()
    {
        // Note that this solution doesn't need to skip the columns that are used by the odd threat,
        // since we will win the game the next turn anyway when using this solution
        for(int col = 0; col < 7; ++col)
        {
            // Only columns that have more than 1 empty square may be usefull
            if(playableCols[col] == -1 || playableCols[col] == 5) continue;

            // Check if there is a level 3 threat above and on the direct playable square
            if(hasLevel3WinningThreat(col, playableCols[col]) && hasLevel3WinningThreat(col, playableCols[col] + 1))
            {
                // If we've got a direct completable threat and one above it we will win our next move
                // The opponent can only fill in one threat and we can win on the other threat
                // Since there are no direct threats of the opponent (this function wouldn't be called if there are) we solve all threats
                ThreatSolution* solution = new ThreatSolution(ThreatSolution::AfterVertical);
                solution->addSquare(col, playableCols[col]);
                solution->addSquare(col, playableCols[col] + 1);
                solution->solvedThreats = std::vector<LineThreat*>(threats.begin(), threats.end());
                for(std::list<LineThreat*>::const_iterator pos = threats.begin(); pos != threats.end(); ++pos)
                    (*pos)->solutions.push_back(solution);
                solutions.push_front(solution);
            }
        }
    }

    bool BoardExt::solutionCanCombine1(const ThreatSolution* s1, const ThreatSolution* s2) const
    { return !s1->interferes(s2); }
    bool BoardExt::solutionCanCombine2(const ThreatSolution* s1, const ThreatSolution* s2) const
    {
        const ThreatSolution* inverse = s1->type == ThreatSolution::LowInverse || s1->type == ThreatSolution::HighInverse ? s1 : s2;
        const ThreatSolution* other = s1->type == ThreatSolution::LowInverse || s1->type == ThreatSolution::HighInverse ? s2 : s1;

        // The squares where the inverse starts
        const int upperRowDelta = inverse->type == ThreatSolution::LowInverse ? 1 : 2;
        PieceCoords start1(-1, -1);
        PieceCoords start2(-1, -1);
        for(unsigned int i = 0; i < inverse->squareCount(); ++i)
        {
            if(start1.col == -1)
                start1 = inverse->at(i);
            else if(start1.col != inverse->at(i).col)
            {
                start2 = inverse->at(i);
                break;
            }
        }

        // Find out if there are any ClaimEvens below one of the starts
        if(other->type == ThreatSolution::ClaimEven)
            return (start1.col != other->at(0).col || start1.row + upperRowDelta < other->at(0).row) && (start2.col != other->at(0).col || start2.row + upperRowDelta < other->at(0).row);
        else if(other->type == ThreatSolution::AfterEven)
        {
            for(unsigned int i = 0; i < other->squareCount(); ++i)
            {
                if(other->at(i).col == start1.col && other->at(i).row <= start1.row + upperRowDelta)
                    return false;
                if(other->at(i).col == start2.col && other->at(i).row <= start2.row + upperRowDelta)
                    return false;
            }
        }
        else if(other->type == ThreatSolution::BaseClaim)
        {
            PieceCoords lowerClaimEvenSquare(-1, 0);
            for(unsigned int i = 0; i < other->squareCount(); ++i)
            {
                if(lowerClaimEvenSquare.col == other->at(i).col)
                    break;
                lowerClaimEvenSquare = other->at(i);
            }
            return (start1.col != lowerClaimEvenSquare.col || start1.row + upperRowDelta < lowerClaimEvenSquare.row) && (start2.col != lowerClaimEvenSquare.col || start2.row + upperRowDelta < lowerClaimEvenSquare.row);
        }
        else if(other->type == ThreatSolution::Before || other->type == ThreatSolution::SpecialBefore)
        {
            const ThreatSolutionBefore* before = dynamic_cast<const ThreatSolutionBefore*>(other);
            if(before == 0) return true;

            for(int col = 0; col < 7; ++col)
            {
                if(!before->claimEvenUsedInCol(col)) continue;

                const int& row = before->lowestUsedSquareInCol(col);
                if((start1.col == col && row <= start1.row + upperRowDelta) || (start2.col == col && row <= start2.row + upperRowDelta))
                    return false;
            }
            return true;
        }

        return true;
    }
    bool BoardExt::solutionCanCombine3(const ThreatSolution* s1, const ThreatSolution* s2) const
    { return !s1->interferesColumnWise(s2); }
    bool BoardExt::solutionCanCombine4(const ThreatSolution* s1, const ThreatSolution* s2) const
    {
        // The sets of squares must be disjoint
        if(s1->interferes(s2)) return false;

        // Find out which columns the inverse s1 uses
        int col1 = -1;
        int col2 = -1;
        for(unsigned int i = 0; i < s1->squareCount(); ++i)
        {
            if(col1 == -1)
                col1 = s1->at(i).col;
            else if(s1->at(i).col != col1)
            {
                col2 = s1->at(i).col;
                break;
            }
        }

        // Check which columns the inverses share
        bool col1Found = false;
        bool col2Found = false;
        for(unsigned int i = 0; i < s2->squareCount(); ++i)
        {
            if(col1 == s2->at(i).col)
            {
                col1Found = true;
                continue;
            }
            else if(!col1Found && s2->at(i).col > col1)
                break;

            if(col2 == s2->at(i).col)
                col2Found = true;
            else if(!col2Found && s2->at(i).col > col2)
                break;
        }

        // Either col1Found and col2Found have both to be true or both should be false
        return (col1Found && col2Found) || (!col1Found && !col2Found);
    }
