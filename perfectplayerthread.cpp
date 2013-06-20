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

#include "perfectplayerthread.h"
#include <ctime>
#include <QMutexLocker>
#include <QFile>

// Public:
    PerfectPlayerThread::PerfectPlayerThread(const bool& isRed)
    : isRed(isRed), board(isRed), keepRunning(false), simulatorsKeepRunning(false), alphaBetaKeepRunning(false)
    {
        qRegisterMetaType<StatusPhase>("MoveSmartness");
    }

    PerfectPlayerThread::~PerfectPlayerThread()
    {
        // Interrupt all running threads
        simulatorsKeepRunning = false;
        alphaBetaKeepRunning = false;

        // Wait for all running threads to exit
        QThreadPool::globalInstance()->waitForDone();
    }

// Public slots:
    void PerfectPlayerThread::setBoard(const Board& b)
    {
        QMutexLocker locker(&board);
        board.changesMade(b);
    }

    void PerfectPlayerThread::searchMove()
    {
        // Wait for any old threads in the ThreadPool to exit
        QThreadPool::globalInstance()->waitForDone();

        // Start the searching
        keepRunning = true;
        simulatorsKeepRunning = true;
        QMutexLocker locker(&board);

        // Find which columns can be played
        if(!keepRunning) return;
        statusUpdate(FindingPlayableCols);
        board.findPlayableCols();

        // If the board is empty and we're red, we'll play the middle square
        bool boardEmpty = true;
        for(int col = 0; col < 7; ++col)
        {
            if(board.playableRow(col) != 0)
            {
                boardEmpty = false;
                break;
            }
        }
        if(boardEmpty)
        {
            // Although we're not really TreeSearching, we're doing something that's necessary for the tree search (building the database)
            statusUpdate(TreeSearching);

            // Load some precalculated positions from the database
            if(!AlphaBetaSearcher::positionDatabaseLoaded())
                AlphaBetaSearcher::loadPositionDatabase();

            doMove(3);
            return;
        }

        // Try to win the game at once
        statusUpdate(TryingWinningMove);
        int move = tryWinningMove();
        if(move != -1 && keepRunning)
        {
            doMove(move);
            return;
        }
        else if(!keepRunning) return;

        // Try to block direct enemy threats
        statusUpdate(BlockingLosingMove);
        move = blockEnemyWinningMove();
        if(move != -1 && keepRunning)
        {
            doMove(move);
            return;
        }
        else if(!keepRunning) return;

        // This list is going to keep track of how smart each possible move is
        simulationResults = std::vector<MoveSmartness>(7, Unknown);
        unsigned int resultCount = 0;

        // Check for each column if solutions can be found
        statusUpdate(SearchingSolutions, 0);
        for(unsigned int col = 0; col < 7; ++col)
        {
            // Check if we're not interrupted
            if(!keepRunning) return;

            // If this column isn't playable, we mark it as impossible
            if(board.playableRow(col) == -1)
            {
                simulationResults[col] = Impossible;
                statusUpdate(SearchingSolutions, ++resultCount);
                continue;
            }

            // Check if playing this column doesn't lead to a direct lose
            if(board.playableRow(col) != 5 && board.hasLevel3Threat(col, board.playableRow(col) + 1))
            {
                simulationResults[col] = DirectLose;
                statusUpdate(SearchingSolutions, ++resultCount);
                continue;
            }

            // Find out if there is a solution for this move
            acceptSimulationDone = true;
            MoveSimulator* simulator = new MoveSimulator(board.doMove(col, isRed ? Red : Yellow), col, isRed, isRed ? AllSolved : AllSolvedWin);
            simulator->setInterruptedPointer(&simulatorsKeepRunning);
            connect(simulator, SIGNAL(done(const int&, const MoveSmartness&)), this, SLOT(simulationDone(const int&, const MoveSmartness&)));
            QThreadPool::globalInstance()->start(simulator);    // QThreadPool will clean up the simulator when it's done
        }

        // If all results have been found we can make the move
        if(resultCount == 7)
        {
            // Check if we're not interrupted
            if(!keepRunning) return;

            // Randomly choose a move from the best moves
            statusUpdate(ChoosingAMove);
            std::vector<unsigned int> cols;
            for(int col = 0; col < 7; ++col)
            {
                if(simulationResults[col] == DirectLose)
                    cols.push_back(col);
            }
            if(cols.empty())
            {
                for(unsigned int col = 0; keepRunning && col < 7; ++col)
                {
                    if(board.playableRow(col) != -1)
                        cols.push_back(col);
                }
            }

            if(keepRunning)
                doMove(cols[qrand() % cols.size()]);
        }
    }

    void PerfectPlayerThread::stop()
    {
        simulatorsKeepRunning = false;
        keepRunning = false;
    }

// Private:
    int PerfectPlayerThread::tryWinningMove()
    {
        // Check if we're not interrupted
        if(!keepRunning) return -1;

        // Find all possible winning threats
        board.searchForWinningThreats();

        // Check if we're not interrupted
        if(!keepRunning) return -1;

        // If there is a move that wins the game directly, we play that move
        for(unsigned int col = 0; keepRunning && col < 7; ++col)
        {
            if(board.playableRow(col) == -1) continue;

            if(board.hasLevel3WinningThreat(col, board.playableRow(col)))
                return col;
        }

        // No move is found
        return -1;
    }

    int PerfectPlayerThread::blockEnemyWinningMove()
    {
        // Check if we're not interrupted
        if(!keepRunning) return -1;

        // Find all possible threats
        board.searchForThreats();

        // Check if we're not interrupted
        if(!keepRunning) return -1;

        // If the enemy could play a move that wins the game for him directly, we play that move before him
        for(unsigned int col = 0; keepRunning && col < 7; ++col)
        {
            if(board.playableRow(col) == -1) continue;

            if(board.hasLevel3Threat(col, board.playableRow(col)))
                return col;
        }

        // No move is found
        return -1;
    }

// Private slots:
    void PerfectPlayerThread::simulationDone(const int& col, const MoveSmartness& result)
    {
        // If we don't accept results anymore, we stop here
        if(!acceptSimulationDone) return;

        // Store the result in the list
        simulationResults[col] = result;

        // Check how many results we have and directly check what the best result is
        unsigned int resultCount = 0;
        MoveSmartness bestMove = Unknown;
        int goodMoveCount = 0;
        for(std::vector<MoveSmartness>::const_iterator pos = simulationResults.begin(); pos != simulationResults.end(); ++pos)
        {
            if(*pos != Unknown)         ++resultCount;
            if(*pos >= NotAllSolved)    ++goodMoveCount;
            if(*pos > bestMove)         bestMove = *pos;
        }

        // Update the status on how many results we've found
        statusUpdate(SearchingSolutions, resultCount);

        // If we haven't got all results yet, we keep looking
        // We can stop looking if we've found a perfect move
        // For yellow this is a move that will win the game
        // For red an AllSolved is enough since this means that red has an odd threat somewhere where it will win
        if(resultCount != 7 && bestMove < (isRed ? AllSolved : AllSolvedWin)) return;
        acceptSimulationDone = false;
        simulatorsKeepRunning = false;

        // Check if we're not interrupted
        if(!keepRunning) return;

        // If we haven't found a winning move, determine the best move using alpha-beta search
        // Note that for red an AllSolved means a win since he will win at the odd threat he has (or maybe sooner)
        if(goodMoveCount > 1 && bestMove < (isRed ? AllSolved : AllSolvedWin))
        {
            // Note that if we've come here all MoveSimulator threads must have quit
            // Because we either received all results and if we stopped earlier because we found a winning move,
            // we wouldn't be here
            // Therefore calling QThreadPool::globalInstance()->waitForDone() is not necessary

            // Update our status
            statusUpdate(TreeSearching, 0);

            // Check if we're not interrupted
            if(!keepRunning) return;

            // Initialize the move database (if it hasn't been initialized already)
            if(!AlphaBetaSearcher::positionDatabaseLoaded())
                AlphaBetaSearcher::loadPositionDatabase();

            // Create a BitBoard
            const BitBoard bitBoard(BitBoard::board2int(board));

            // Check if we're not interrupted
            if(!keepRunning) return;

            // We will accept results from the alpha-beta searchers and will also keep track of their results
            alphaBetaKeepRunning = true;
            acceptAlphaBetaResults = true;
            alphaBetaResults.clear();

            // Start a thread for each move to solve it using alpha-beta search
            for(int col = 0; col < 7; ++col)
            {
                // If the move leads to a defeat or is impossible, there is no use in using alpha-beta search on it
                if(simulationResults[col] < NotAllSolved) continue;

                // Check if we're not interrupted
                if(!keepRunning) return;

                // We remember that we still expect a result from this move
                alphaBetaResults[col] = AlphaBetaResult();

                // Try to solve the chosen move
                const BitBoard newBoard = bitBoard.move(col);
                AlphaBetaSearcher* searcher = new AlphaBetaSearcher(newBoard, col);
                searcher->setInterruptedPointer(&alphaBetaKeepRunning);
                connect(searcher, SIGNAL(done(const int&, const quint16&)), this, SLOT(alphaBetaDone(const int&, const quint16&)));
                QThreadPool::globalInstance()->start(searcher);    // QThreadPool will clean up the searcher when it's done
            }

            // Stop here
            return;
        }

        // Randomly choose a move from the best moves
        qsrand(time(0));
        statusUpdate(ChoosingAMove);
        std::vector<unsigned int> cols;
        for(int col = 0; col < 7; ++col)
        {
            if(simulationResults[col] == bestMove)
                cols.push_back(col);
        }
        if(cols.empty())
        {
            for(unsigned int col = 0; keepRunning && col < 7; ++col)
            {
                if(board.playableRow(col) != -1)
                    cols.push_back(col);
            }
        }

        if(keepRunning)
            doMove(cols[qrand() % cols.size()]);
    }

    void PerfectPlayerThread::alphaBetaDone(const int& col, const quint16& val)
    {
        // If we don't accept alpha-beta results, we stop here
        if(!acceptAlphaBetaResults) return;

        // Add the result
        alphaBetaResults[col].reported = true;
        alphaBetaResults[col].result = val;

        // Count the results
        unsigned int resultCount = 0;
        AlphaBetaSearcher::PositionValue bestValue = isRed ? AlphaBetaSearcher::Loss : AlphaBetaSearcher::Win;
        for(std::map<int, AlphaBetaResult>::const_iterator pos = alphaBetaResults.begin(); pos != alphaBetaResults.end(); ++pos)
        {
            if(pos->second.reported)
            {
                // Increase the result count
                ++resultCount;

                // The game theoretical value of the result in pos
                const quint16 currVal = AlphaBetaSearcher::getValue(pos->second.result);

                // An unknown value is still better than losing
                if(currVal == AlphaBetaSearcher::ValueUnknown ?
                    bestValue == (isRed ? AlphaBetaSearcher::Loss : AlphaBetaSearcher::Win) :
                    (isRed ? currVal > bestValue : currVal < bestValue))
                {
                    bestValue = currVal;
                }
            }
        }

        // Update our status
        statusUpdate(TreeSearching, 100 * resultCount / alphaBetaResults.size());

        // If we're only working on one move, and all moves untill now mean a loss we just choose that move
        // It can't have a worse outcome and it probably has a greater or somewhat equal depth since it's the last thread to report a result
        if(resultCount == alphaBetaResults.size() - 1 && bestValue == (isRed ? AlphaBetaSearcher::Loss : AlphaBetaSearcher::Win))
        {
            // Stop the remaining thread
            acceptAlphaBetaResults = false;
            alphaBetaKeepRunning = false;

            // Do the move
            if(keepRunning)
            {
                for(std::map<int, AlphaBetaResult>::const_iterator pos = alphaBetaResults.begin(); pos != alphaBetaResults.end(); ++pos)
                {
                    if(!pos->second.reported)
                        doMove(pos->first);
                }
            }
            return;
        }

        // If not all results have been found and no winning move is found, we have to continue searching
        if(resultCount != alphaBetaResults.size() && AlphaBetaSearcher::getValue(val) != (isRed ? AlphaBetaSearcher::Win : AlphaBetaSearcher::Loss))
            return;

        // Update our status
        statusUpdate(ChoosingAMove);

        // Since we've all the results we want, we can stop searching
        acceptAlphaBetaResults = false;
        alphaBetaKeepRunning = false;

        // If we just found the winning move, we do that move and stop searching
        if(AlphaBetaSearcher::getValue(val) == isRed ? AlphaBetaSearcher::Win : AlphaBetaSearcher::Loss)
        {
            if(keepRunning)
                doMove(col);
            return;
        }

        // Loop through all moves and choose the best one
        quint16 bestDepth = 0;
        int bestCol = 0;
        for(std::map<int, AlphaBetaResult>::const_iterator pos = alphaBetaResults.begin(); pos != alphaBetaResults.end(); ++pos)
        {
            const quint16 currVal = AlphaBetaSearcher::getValue(pos->second.result);
            const quint16 currDepth = AlphaBetaSearcher::getDepth(pos->second.result);

            // We already know what the best value is, just search the one with the greatest depth
            // Because since we can't make a winning move, we choose the move where it takes the longest to finish the game
            // By doing so we maximize the chance of the opponent making a mistake
            if(bestValue == currVal && currDepth > bestDepth)
            {
                bestDepth = currDepth;
                bestCol = pos->first;
            }
        }

        // Do the best move
        if(keepRunning)
            doMove(bestCol);
        return;
    }
