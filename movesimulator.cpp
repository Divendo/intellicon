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

#include "movesimulator.h"
#include <QMutexLocker>

// Public:
    MoveSimulator::MoveSimulator(const Board& board, const int& move, const bool& isRed, const MoveSmartness& leastAchievement)
    : board(board, isRed), move(move), isRed(isRed), keepRunning(0), leastAchievement(leastAchievement), tryYellowSolve(true)
    { setAutoDelete(true); }

    void MoveSimulator::run()
    {
        MoveSmartness result = simulate();
        if(keepRunning != 0 && *keepRunning)
            done(move, result);
    }


    void MoveSimulator::setInterruptedPointer(const bool* p)
    { keepRunning = p; }

    void MoveSimulator::dontTryYellowSolve()
    { tryYellowSolve = false; }

// Public slots:
    MoveSmartness MoveSimulator::simulate()
    {
        // Find playable columns and threats
        board.findPlayableCols();
        board.searchForWinningThreats();
        board.searchForThreats();

        // Check if we're not interrupted
        if(keepRunning != 0 && !*keepRunning) return Unknown;

        // If we're red we can only apply the strategic rules if we've an odd threat
        if(!isRed || board.hasOddThreat())
        {
            // Mark threats as solved that are solved by the odd threats we have (but only if we're red)
            if(isRed)
                board.solveByOddThreats();

            // Check if we're not interrupted
            if(keepRunning != 0 && !*keepRunning) return Unknown;

            // Find all possible solutions
            board.searchSolutions();

            // Check if we're not interrupted
            if(keepRunning != 0 && !*keepRunning) return Unknown;

            // Try to find a set of solutions
            const MoveSmartness result = findSolutionSet(board.threats, board.solutions);
            if(keepRunning != 0 && *keepRunning) return result;
        }
        else if(keepRunning != 0 && *keepRunning)
        {
            // First check if yellow could solve this position
            if(tryYellowSolve && board.pieceCount() > 8)
            {
                for(unsigned int col = 0; col < 7; ++col)
                {
                    // Check if we're not interrupted
                    if(keepRunning != 0 && !*keepRunning) return Unknown;

                    // If this column isn't playable, we mark it as impossible
                    if(board.playableRow(col) == -1)
                        continue;

                    // Check if the opponent can complete a group
                    if(board.hasLevel3Threat(col, board.playableRow(col)))
                        return NotAllSolved;

                    // Check if playing this column doesn't lead to a direct lose (in which case we won't play it)
                    if(board.playableRow(col) != 5 && board.hasLevel3Threat(col, board.playableRow(col) + 1))
                        continue;

                    // Find out if there is a solution for this move
                    MoveSimulator simulator(board.doMove(col, Yellow), col, false, AllSolved);
                    if(simulator.simulate() >= AllSolved)
                        return NotAllSolved;
                }
            }
            return NeedsTreeSearch;
        }
        return Unknown;
    }

// Private:
    MoveSmartness MoveSimulator::findSolutionSet(std::list<LineThreat*> threats, const std::list<ThreatSolution*>& solutions)
    {
        // Check if we're not interrupted
        if(keepRunning != 0 && !*keepRunning) return Unknown;

        // Find out which threat is the hardest to solve
        std::list<LineThreat*>::iterator hardestThreat = threats.end();
        unsigned int solutionCount = 0;
        unsigned int tmpCount;
        for(std::list<LineThreat*>::iterator pos = threats.begin(); pos != threats.end(); ++pos)
        {
            if((*pos)->solved || (*pos)->solvedTemp) continue;

            if(hardestThreat == threats.end() || (*pos)->solutions.size() < solutionCount)
            {
                hardestThreat = pos;
                solutionCount = 0;
                for(std::list<ThreatSolution*>::const_iterator pos2 = (*pos)->solutions.begin(); pos2 != (*pos)->solutions.end(); ++pos2)
                {
                    if(!(*pos2)->dontUse) ++solutionCount;
                }
            }
            else
            {
                tmpCount = 0;
                for(std::list<ThreatSolution*>::const_iterator pos2 = (*pos)->solutions.begin(); pos2 != (*pos)->solutions.end(); ++pos2)
                {
                    if(!(*pos2)->dontUse)
                    {
                        if(++tmpCount > solutionCount) break;
                    }
                }
                if(tmpCount < solutionCount)
                {
                    hardestThreat = pos;
                    solutionCount = tmpCount;
                }
            }

            if(solutionCount == 0) break;
        }

        // Check if we're not interrupted
        if(keepRunning != 0 && !*keepRunning) return Unknown;

        // If there is no hardest threat, we're done searching so we've found a solution
        if(hardestThreat == threats.end())
        {
            if(leastAchievement == AllSolvedWin)
            {
                for(std::list<ThreatSolution*>::const_iterator pos = solutions.begin(); pos != solutions.end(); ++pos)
                {
                    if((*pos)->dontUse) continue;
                    if((*pos)->winsGame())
                        return AllSolvedWin;
                }
            }
            return AllSolved;
        }

        // If the hardest threat can't be solved we stop searching
        if(solutionCount == 0)
            return NotAllSolved;

        // Remove the threat from the list, effectively marking it as solved
        LineThreat* threat = *hardestThreat;
        threats.erase(hardestThreat);
        threat->solvedTemp = true;

        // Try all solutions of the hardest threat
        MoveSmartness best = NotAllSolved;
        MoveSmartness result;
        for(std::list<ThreatSolution*>::iterator pos = threat->solutions.begin(); pos != threat->solutions.end(); ++pos)
        {
            // Check if we're not interrupted
            if(keepRunning != 0 && !*keepRunning) return Unknown;

            if((*pos)->dontUse) continue;

            // Mark all neighbours as unusable
            std::vector<ThreatSolution*> cantCombineAdded;
            cantCombineAdded.reserve((*pos)->cantCombine.size());
            for(std::vector<ThreatSolution*>::iterator pos2 = (*pos)->cantCombine.begin(); pos2 != (*pos)->cantCombine.end(); ++pos2)
            {
                if((*pos2)->dontUse) continue;
                (*pos2)->dontUse = true;
                cantCombineAdded.push_back(*pos2);
            }

            // Mark all connected threats temporarily as solved
            std::vector<LineThreat*> threatsTempSolved;
            threatsTempSolved.reserve((*pos)->solvedThreats.size());
            for(std::vector<LineThreat*>::iterator pos2 = (*pos)->solvedThreats.begin(); pos2 != (*pos)->solvedThreats.end(); ++pos2)
            {
                if((*pos2)->solvedTemp) continue;
                (*pos2)->solvedTemp = true;
                threatsTempSolved.push_back(*pos2);
            }

            // Try to find a solution for the new set of threats, using the new set of solutions
            result = findSolutionSet(threats, solutions);

            // Remove the temporarily solve from the threats
            for(std::vector<LineThreat*>::iterator pos2 = threatsTempSolved.begin(); pos2 != threatsTempSolved.end(); ++pos2)
                (*pos2)->solvedTemp = false;

            // Mark all neighbours as usable again
            for(std::vector<ThreatSolution*>::iterator pos2 = cantCombineAdded.begin(); pos2 != cantCombineAdded.end(); ++pos2)
                (*pos2)->dontUse = false;

            // If the current result is better than any of the previous results we will use this result
            if(result > best)
            {
                if((best = result) >= leastAchievement) break;  // If we found a solution, no more searching is needed
            }
        }

        threat->solvedTemp = false;

        return best;
    }
