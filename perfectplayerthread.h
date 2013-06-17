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
#include <QHash>
#include "boardext.h"
#include "movesimulator.h"
#include "bitboard.h"

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

    signals:
        void doMove(const int& col);
        void statusUpdate(const StatusPhase& phase, const int& n = -1);
        
    public slots:
        void setBoard(const Board& b);
        void searchMove();
        void stop();

    private:
        bool isRed;                     // The color of the player
        bool keepRunning;               // Whether we should keep searching for moves (true) or are interrupted (false)
        bool simulatorsKeepRunning;     // Whether the simulators should keep searching for a solution (true) or not (false)
        BoardExt board;                 // The board (extended version that can search for threats etc)
        bool acceptSimulationDone;      // Whether to accept incoming results from simulations

        /// These functions return -1 if no move is found, if a move is found the column of the move is returned
        // Tries to find a move that directly wins the game
        int tryWinningMove();
        // Tries to block any direct threat of the enemy that would win the game otherwise
        int blockEnemyWinningMove();

        // Returns per column whether all threats can be solved if we'd move there
        std::vector<MoveSmartness> tryMoves();

        std::vector<MoveSmartness> results;         // The results of the move simulations

        /// Functions for the search tree database
        // PositionValue:
        //  The first 3 bits in the unsigned int tell us the value of that position
        //  The remaining bits tell us how deep we went (from that position on) to determine that value
        typedef quint16 PositionValue;
        const PositionValue ValueUnknown;
        const PositionValue Loss;
        const PositionValue DrawLoss;
        const PositionValue Draw;
        const PositionValue DrawWin;
        const PositionValue Win;
        // Creates a PositionValue
        PositionValue createPositionValue(const PositionValue& val, const quint16& depth);
        // Get the value part of a PositionValue
        PositionValue getValue(const PositionValue& val);
        // Get how deep we went for this PositionValue
        quint16 getDepth(const PositionValue& val);

        // Positions of which the value is known
        static QHash<quint64, PositionValue> knownPositions[];
        // Load the known position from the database
        void loadPositionDatabase();
        // Used for the history heuristic
        int historyHeuristic[2][42];
        // Initialise the history heuristic array
        void initHistoryHeuristic();
        // Finds the value of the given position
        PositionValue alphaBeta(const quint64& bitBoard, const quint64& redBoard, const quint64& yellowBoard, PositionValue alpha, PositionValue beta);

    private slots:
        void simulationDone(const int& col, const MoveSmartness& result);
};

#endif // PERFECTPLAYERTHREAD_H
