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
    : isRed(isRed), keepRunning(false), simulatorsKeepRunning(false), board(isRed),
    ValueUnknown(0), Loss(1), DrawLoss(2), Draw(3), DrawWin(4), Win(5)
    {
        qRegisterMetaType<StatusPhase>("MoveSmartness");

        // Initialise the historyHeuristic array
        initHistoryHeuristic();
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
            if(PerfectPlayerThread::knownPositions[8].empty())
                loadPositionDatabase();

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
        results = std::vector<MoveSmartness>(7, Unknown);
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
                results[col] = Impossible;
                statusUpdate(SearchingSolutions, ++resultCount);
                continue;
            }

            // Check if playing this column doesn't lead to a direct lose
            if(board.playableRow(col) != 5 && board.hasLevel3Threat(col, board.playableRow(col) + 1))
            {
                results[col] = DirectLose;
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
                if(results[col] == DirectLose)
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

    PerfectPlayerThread::PositionValue PerfectPlayerThread::createPositionValue(const PositionValue& val, const quint16& depth)
    {
        // Lower 3 bits are the value
        // The rest of the bits represent the depth
        return val | (depth << 3);
    }

    PerfectPlayerThread::PositionValue PerfectPlayerThread::getValue(const PositionValue& val)
    {
        // 7 is in binary: 111
        // So we only get the lowest 3 bits
        return val & static_cast<quint16>(7);
    }

    quint16 PerfectPlayerThread::getDepth(const PositionValue& val)
    {
        // Throw away the 3 lowest bits
        return val >> 3;
    }

    QHash<quint64, PerfectPlayerThread::PositionValue> PerfectPlayerThread::knownPositions[] =
    {
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(),
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(),
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(),
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(),
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(),
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(),
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(),
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(),
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(),
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(),
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(),
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(),
        QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>(), QHash<quint64, PerfectPlayerThread::PositionValue>()
        // Since the minimal required depth to store something in the database is 4 none of the last 3 positions will ever get in the database
    };

    void PerfectPlayerThread::loadPositionDatabase()
    {
        // Open the file
        QFile file(":/data/positions.db");
        file.open(QIODevice::ReadOnly);

        // Reserve the exact size needed for the database
        PerfectPlayerThread::knownPositions[8].reserve(67557);

        // Read the precalculated database
        QByteArray line;
        while(!file.atEnd())
        {
            // Check if we're not interrupted
            if(!keepRunning)
            {
                PerfectPlayerThread::knownPositions[8].clear();
                return;
            }

            line = file.readLine();
            if(line.size() != 44) break;

            const char& value = line.at(line.size() - 2);
            const BitBoard bitBoard = BitBoard(BitBoard::board2int(line.left(42).data()));
            const quint64 dbPosition = qMin(bitBoard.toInt(), bitBoard.flip());
            PerfectPlayerThread::knownPositions[8][dbPosition] = createPositionValue(value == '2' ? Win : (value == '1' ? Draw : Loss), 0);
        }
    }

    void PerfectPlayerThread::initHistoryHeuristic()
    {
        // Describes the dimensions of the board
        const int width = 7;
        const int height = 6;

        // Fills every square in the historyHeuristic array with the number of possible threats on that square
        for(int side = 0; side < 2; ++side)
        {
            for(int col = 0; col < (width + 1) / 2; ++col)
            {
                for(int row = 0; row < (height + 1) / 2; ++row)
                {
                    historyHeuristic[side][height * col + row] =
                    historyHeuristic[side][height * (width - 1 - col) + height - 1 - row] =
                    historyHeuristic[side][height * col + height - 1 - row] =
                    historyHeuristic[side][height * (width - 1 - col) + row] =
                    4+qMin(3,col) + qMax(-1,qMin(3,row)-qMax(0,3-col)) + qMin(3,qMin(col,row)) + qMin(3,row);
                }
            }
        }
    }

    PerfectPlayerThread::PositionValue PerfectPlayerThread::alphaBeta(const BitBoard& bitBoard, PositionValue alpha, PositionValue beta)
    {
        // The amount of pieces on the board
        const int pieceCount = bitBoard.pieceCount();

        // The position that should be used as index in the database
        const quint64 dbPosition = qMin(bitBoard.toInt(), bitBoard.flip());

        // First we try to look up the value of this position in our database
        if(pieceCount >= 8 && pieceCount < 38 && PerfectPlayerThread::knownPositions[pieceCount].contains(dbPosition))
        {
            const PositionValue& posVal = PerfectPlayerThread::knownPositions[pieceCount][dbPosition];
            const PositionValue val = getValue(posVal);

            // If the value isn't clear because a cutoff occurred we may want to sort out which value it has
            if(val == DrawWin || val == DrawLoss)
            {
                // If the parent node would choose this move anyway, no further evaluation is needed
                if(bitBoard.redToMove() ? val < beta : val > alpha)
                    return createPositionValue(val, 1 + getDepth(posVal));
            }
            else
                return createPositionValue(val, 1 + getDepth(posVal));
        }

        // If there are exactly 8 pieces on the board the database should have provided us with a value
        // If no value was found in the database, then we're playing a forced move
        // We see it as a Draw
        if(pieceCount == 8)
            return createPositionValue(ValueUnknown, 0);

        // If the board is full, it's a draw
        if(bitBoard.isFull())
            return createPositionValue(Draw, 0);

        // Check if we're not interrupted
        if(!keepRunning) return createPositionValue(ValueUnknown, 0);

        // Check if the opponent can win or if we have a forced move
        // Also check which moves would make it possible for the opponent to win directly (and so we don't play them)
        std::vector<int> moves;
        moves.reserve(7);
        const quint64 other = bitBoard.redToMove() ? bitBoard.yellowToInt() : bitBoard.redToInt();
        for(int col = 0; col < 7; ++col)
        {
            if(!bitBoard.canMove(col)) continue;
            const int row = bitBoard.playableRow(col);

            // Check if the opponent can win on the square above the playable square in this column
            const bool winOnTop = BitBoard::isWinner(other | (Q_UINT64_C(2) << (row + 7 * col)));

            // Check if the opponent can win directly by playing this column
            if(BitBoard::isWinner(other | (Q_UINT64_C(1) << (row + 7 * col))))
            {
                // A double threat can't be stopped
                if(winOnTop)
                    return createPositionValue(bitBoard.redToMove() ? Loss : Win, 0);

                // It's a forced move
                moves.clear();
                moves.push_back(col);

                // If another forced move is found, we can't stop the opponent from winning
                while(++col < 7)
                {
                    if(BitBoard::isWinner(other | (Q_UINT64_C(1) << (bitBoard.playableRow(col) + 7 * col))))
                        return createPositionValue(bitBoard.redToMove() ? Loss : Win, 0);
                }

                // Stop looking for any other moves
                break;
            }

            // Only play this move if the opponent wouldn't win by it directly
            if(!winOnTop)
                moves.push_back(col);
        }

        // Check if we're not interrupted
        if(!keepRunning) return createPositionValue(ValueUnknown, 0);

        // If no moves were found, we lose
        if(moves.empty())
            return createPositionValue(bitBoard.redToMove() ? Loss : Win, 0);

        // Find a value for each move
        const unsigned int moveCount = moves.size();
        bool valUnknown = false;
        PositionValue bestScore = bitBoard.redToMove() ? Loss : Win;
        quint16 bestDepth = 0;
        for(unsigned int move = 0; move < moveCount; ++move)
        {
            // Check if we're not interrupted
            if(!keepRunning) return createPositionValue(ValueUnknown, 0);

            // Dynamically order the moves using the historyHeuristic board
            int row = bitBoard.playableRow(moves[move]);
            int bestHistory = historyHeuristic[bitBoard.redToMove()][6 * moves[move] + row];
            unsigned int bestMoveIndex = move;
            for(unsigned int i = move + 1; i < moveCount; ++i)
            {
                const int r = bitBoard.playableRow(moves[i]);
                if(historyHeuristic[bitBoard.redToMove()][6 * moves[i] + r] > bestHistory)
                {
                    row = r;
                    bestHistory = historyHeuristic[bitBoard.redToMove()][6 * moves[i] + r];
                    bestMoveIndex = i;
                }
            }
            const int bestMoveCol = moves[bestMoveIndex];
            for(; bestMoveIndex > move; --bestMoveIndex)
                moves[bestMoveIndex] = moves[bestMoveIndex - 1];
            moves[move] = bestMoveCol;

            // Make the move
            PositionValue posVal = alphaBeta(bitBoard.move(bestMoveCol), alpha, beta);
            PositionValue val = getValue(posVal);

            // Check if we're not interrupted
            if(!keepRunning) return createPositionValue(ValueUnknown, 0);

            if(val == ValueUnknown)
                valUnknown = true;
            else
            {
                // Set the alpha/beta
                if(bitBoard.redToMove())
                {
                    if(val > bestScore)
                    {
                        bestScore = val;
                        bestDepth = getDepth(posVal);

                        if(val > alpha)
                            alpha = val;
                    }
                    else if(val == bestScore && getDepth(posVal) > bestDepth)
                        bestDepth = getDepth(posVal);
                }
                else
                {
                    if(val < bestScore)
                    {
                        bestScore = val;
                        bestDepth = getDepth(posVal);

                        if(val < beta)
                            beta = val;
                    }
                    else if(val == bestScore && getDepth(posVal) > bestDepth)
                        bestDepth = getDepth(posVal);
                }

                // Check if we can make a cutoff
                if(beta <= alpha)
                {
                    // Since we've cut off a part of the tree we increase the history score of this move
                    if(move != 0)
                    {
                        // Punish badly chosen moves
                        for(unsigned int i = 0; i < move; ++i)
                            --historyHeuristic[bitBoard.redToMove()][6 * moves[i] + bitBoard.playableRow(moves[i])];

                        // Reward the good chosen move
                        historyHeuristic[bitBoard.redToMove()][6 * moves[move] + bitBoard.playableRow(moves[move])] += move;
                    }

                    // If we do a cutoff at a Draw position it may also be a Win or Loss
                    // But only if not all children have been evaluated yet
                    if(val == Draw && move != moveCount)
                        val = bitBoard.redToMove() ? DrawWin : DrawLoss;

                    // Create our result
                    const PositionValue out = createPositionValue(val, 1 + bestDepth);

                    // Only store the position in the database if there are more than 8 pieces on the board
                    // Also only store positions where red is to move (to reduce the database size)
                    // Also only store positions that took a lot of work
                    if(pieceCount > 8 && bestDepth > 3)
                        PerfectPlayerThread::knownPositions[pieceCount][dbPosition] = out;

                    return out;
                }
            }
        }

        // Check if we're not interrupted
        if(!keepRunning) return createPositionValue(ValueUnknown, 0);

        // If a ValueUnknown was encountered, the value of this position is unknown
        if(valUnknown)
            return createPositionValue(ValueUnknown, 0);

        // Create our result
        const PositionValue out = createPositionValue(bestScore, 1 + bestDepth);

        // Only store the position in the database if there are more than 8 pieces on the board
        // Also only store positions where red is to move (to reduce the database size)
        // Also only store positions that took a lot of work
        if(pieceCount > 8 && bestDepth > 3)
            PerfectPlayerThread::knownPositions[pieceCount][dbPosition] = out;

        return out;
    }

// Private slots:
    void PerfectPlayerThread::simulationDone(const int& col, const MoveSmartness& result)
    {
        // If we don't accept results anymore, we stop here
        if(!acceptSimulationDone) return;

        // Store the result in the list
        results[col] = result;

        // Check how many results we have and directly check what the best result is
        unsigned int resultCount = 0;
        MoveSmartness bestMove = Unknown;
        int bestMoveCount = 0;
        bool hasTreeSearchMoves = false;
        for(std::vector<MoveSmartness>::const_iterator pos = results.begin(); pos != results.end(); ++pos)
        {
            if(*pos != Unknown) ++resultCount;
            if(*pos == bestMove) ++bestMoveCount;
            if(*pos > bestMove)
            {
                bestMove = *pos;
                bestMoveCount = 1;
            }
            if(*pos == NeedsTreeSearch) hasTreeSearchMoves = true;
        }

        // Update the status on how many results we've found
        statusUpdate(SearchingSolutions, resultCount);

        // If we haven't got all results yet, we keep looking
        // We can stop looking if we've found a perfect move
        // For yellow this is a move that will win the game
        // For red an AllSolved is enough since this means that red has an odd threat somewhere where it will win
        if(resultCount != 7 && bestMove != (isRed ? AllSolved : AllSolvedWin)) return;
        acceptSimulationDone = false;
        simulatorsKeepRunning = false;

        // Check if we're not interrupted
        if(!keepRunning) return;

        // If there are some moves that may be solved by a tree search and we haven't found a winning move we'll perform a tree search
        // Note that for red an AllSolved means a win since he will win at the odd threat he has (or maybe sooner)
        if(bestMoveCount > 1 && ((isRed && hasTreeSearchMoves && bestMove < AllSolved) || (!isRed && bestMove < AllSolvedWin && bestMove >= NotAllSolved)))
        {
            // Update our status
            statusUpdate(TreeSearching, 0);

            // Check if we're not interrupted
            if(!keepRunning) return;

            // Initialize the move database (if it hasn't been initialized already)
            if(PerfectPlayerThread::knownPositions[8].empty())
                loadPositionDatabase();

            // Create a BitBoard
            const BitBoard bitBoard(BitBoard::board2int(board));

            // Check if we're not interrupted
            if(!keepRunning) return;

            // Check if the current position is known in the database, if so we know what the best is that we can do
            const quint64 dbPosition = qMin(bitBoard.toInt(), bitBoard.flip());     // The position that should be used as index in the database
            const int pieceCount = bitBoard.pieceCount();                           // The amount of pieces on the board
            PositionValue currPosVal =  ValueUnknown;
            if(pieceCount >= 8 && pieceCount < 38 && PerfectPlayerThread::knownPositions[pieceCount].contains(dbPosition))
                currPosVal = getValue(PerfectPlayerThread::knownPositions[pieceCount][dbPosition]);

            // Check if we're not interrupted
            if(!keepRunning) return;

            // Determine which moves we should check
            std::vector<int> moves;
            moves.reserve(7);
            for(int col = 0; col < 7; ++col)
            {
                if(results[col] == (isRed ? NeedsTreeSearch : bestMove))
                    moves.push_back(col);
            }

            // Try to solve each move
            const unsigned int moveCount = moves.size();
            int bestOptionCol = moves[0];
            int bestOptionDepth = 0;
            PositionValue alpha = !isRed && currPosVal != ValueUnknown ? currPosVal : Loss;
            PositionValue beta = isRed && currPosVal != ValueUnknown ? currPosVal : Win;
            for(unsigned int move = 0; move < moveCount; ++move)
            {
                // Check if we're not interrupted
                if(!keepRunning) return;

                // Use history heuristic to determine which move we could best choose
                int row = bitBoard.playableRow(moves[move]);
                int bestHistory = historyHeuristic[isRed][6 * moves[move] + row];
                unsigned int bestMoveIndex = move;
                for(unsigned int i = move + 1; i < moveCount; ++i)
                {
                    const int r = bitBoard.playableRow(moves[i]);
                    if(historyHeuristic[isRed][6 * moves[i] + r] > bestHistory)
                    {
                        row = r;
                        bestHistory = historyHeuristic[isRed][6 * moves[i] + r];
                        bestMoveIndex = i;
                    }
                }
                const int bestMoveCol = moves[bestMoveIndex];
                for(; bestMoveIndex > move; --bestMoveIndex)
                    moves[bestMoveIndex] = moves[bestMoveIndex - 1];
                moves[move] = bestMoveCol;

                // Try to solve the chosen move
                const PositionValue posVal = alphaBeta(bitBoard.move(bestMoveCol), alpha, beta);
                const PositionValue val = getValue(posVal);
                const quint16 depth = getDepth(posVal);

                // Update our status
                statusUpdate(TreeSearching, 100 * (move + 1) / moveCount);

                // Unknown values are useless
                if(val == ValueUnknown)
                    continue;

                // Update alpha/beta
                if(isRed)
                {
                    if(val > alpha)
                    {
                        bestOptionCol = bestMoveCol;
                        bestOptionDepth = depth;
                        alpha = val;
                    }
                    else if(val == alpha && depth > bestOptionDepth)
                    {
                        bestOptionCol = bestMoveCol;
                        bestOptionDepth = depth;
                    }
                }
                else
                {
                    if(val < beta)
                    {
                        bestOptionCol = bestMoveCol;
                        bestOptionDepth = depth;
                        beta = val;
                    }
                    else if(val == beta && depth > bestOptionDepth)
                    {
                        bestOptionCol = bestMoveCol;
                        bestOptionDepth = depth;
                    }
                }

                // Check if we're not interrupted
                if(!keepRunning) return;

                // Check if we can make do cutoff
                if(beta <= alpha)
                {
                    // We may want to punish/reward certain parts of the historyHeuristic
                    if(move != 0)
                    {
                        // Punish badly chosen moves
                        for(unsigned int i = 0; i < move; ++i)
                            --historyHeuristic[isRed][6 * moves[i] + bitBoard.playableRow(moves[i])];

                        // Reward the good chosen move
                        historyHeuristic[isRed][6 * moves[move] + bitBoard.playableRow(moves[move])] += move;
                    }

                    // Cutoff
                    break;
                }
            }

            // Choose the best move
            if(keepRunning)
                doMove(bestOptionCol);
            return;
        }

        // Randomly choose a move from the best moves
        qsrand(time(0));
        statusUpdate(ChoosingAMove);
        std::vector<unsigned int> cols;
        for(int col = 0; col < 7; ++col)
        {
            if(results[col] == bestMove)
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
