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

#ifndef PERFECTPLAYERTHREAD_H
#define PERFECTPLAYERTHREAD_H

#include <QObject>
#include <QThreadPool>
#include <map>
#include "boardext.h"
#include "movesimulator.h"
#include "bitboard.h"
#include "alphabetasearcher.h"

enum StatusPhase
{
    FindingPlayableCols,
    TryingWinningMove,
    BlockingLosingMove,
    SearchingSolutions,
    TreeSearching,
    ChoosingAMove
};
Q_DECLARE_METATYPE(StatusPhase)

class PerfectPlayerThread : public QObject
{
    Q_OBJECT

    public:
        PerfectPlayerThread(const bool& isRed);
        ~PerfectPlayerThread();

    signals:
        void doMove(const int& col);
        void statusUpdate(const StatusPhase& phase, const int& n = -1);
        
    public slots:
        void setBoard(const Board& b);
        void searchMove();
        void stop();

    private:
        bool isRed;                     // The color of the player
        BoardExt board;                 // The board (extended version that can search for threats etc)
        bool keepRunning;               // Whether we should keep searching for moves (true) or are interrupted (false)
        bool simulatorsKeepRunning;     // Whether the simulators should keep searching for a solution (true) or not (false)
        bool acceptSimulationDone;      // Whether to accept incoming results from simulations
        bool alphaBetaKeepRunning;      // Whether we should keep searching for moves using alpha-beta (true) or if we interupt that search (false)
        bool acceptAlphaBetaResults;    // Whether we accept incoming results from the alpha-beta search

        /// These functions return -1 if no move is found, if a move is found the column of the move is returned
        // Tries to find a move that directly wins the game
        int tryWinningMove();
        // Tries to block any direct threat of the enemy that would win the game otherwise
        int blockEnemyWinningMove();

        /// A struct that represents the result of an AlphaBetaSearcher
        /// The result consists out of the result reported by the searcher and a flag indicating whether the result has been reported
        struct AlphaBetaResult
        {
            bool reported;
            AlphaBetaSearcher::PositionValue result;

            AlphaBetaResult()
            : reported(false), result(AlphaBetaSearcher::ValueUnknown) {}
        };

        std::vector<MoveSmartness> simulationResults;       // The results of the move simulations
        std::map<int, AlphaBetaResult> alphaBetaResults;  // The results of the alpha-beta searches

    private slots:
        void simulationDone(const int& col, const MoveSmartness& result);
        void alphaBetaDone(const int& col, const quint16& result);
};

#endif // PERFECTPLAYERTHREAD_H
