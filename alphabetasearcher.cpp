#include "alphabetasearcher.h"

#include <QFile>

// Public:
    // Static:
        const AlphaBetaSearcher::PositionValue AlphaBetaSearcher::ValueUnknown = 0;
        const AlphaBetaSearcher::PositionValue AlphaBetaSearcher::Loss         = 1;
        const AlphaBetaSearcher::PositionValue AlphaBetaSearcher::DrawLoss     = 2;
        const AlphaBetaSearcher::PositionValue AlphaBetaSearcher::Draw         = 3;
        const AlphaBetaSearcher::PositionValue AlphaBetaSearcher::DrawWin      = 4;
        const AlphaBetaSearcher::PositionValue AlphaBetaSearcher::Win          = 5;

    AlphaBetaSearcher::AlphaBetaSearcher(const BitBoard& board, const int& move)
    : board(board), move(move), keepRunning(0)
    { initHistoryHeuristic(); }

    void AlphaBetaSearcher::setInterruptedPointer(const bool* p)
    { keepRunning = p; }

    void AlphaBetaSearcher::run()
    {
        const PositionValue result = alphaBeta(board.toInt(), board.redToInt(), board.yellowToInt(), Loss, Win);
        if(keepRunning != 0 && *keepRunning)
            done(move, result);
    }

    AlphaBetaSearcher::PositionValue AlphaBetaSearcher::alphaBeta(const quint64& bitBoard, const quint64& redBoard, const quint64& yellowBoard, PositionValue alpha, PositionValue beta)
    {
        // The amount of pieces on the board
        const int pieceCount = BitBoard::pieceCount(redBoard, yellowBoard);

        // Whose turn it is
        const bool redToMove = pieceCount % 2 == 0;

        // The position that should be used as index in the database
        const quint64 dbPosition = qMin(bitBoard, BitBoard::flip(bitBoard));

        // First we try to look up the value of this position in our database
        if(pieceCount >= 8 && pieceCount <= 39 && (pieceCount == 8 || pieceCount % 3 == 0))
        {
            // The database we're going to use
            const int dbIndex = pieceCount / 3 - 2;

            // Lock for reading
            QReadLocker locker(AlphaBetaSearcher::posDbLockers[dbIndex]);

            // Check the database
            if(AlphaBetaSearcher::posDb[dbIndex].contains(dbPosition))
            {
                const PositionValue& posVal = AlphaBetaSearcher::posDb[dbIndex][dbPosition];
                const PositionValue val = getValue(posVal);

                // If the value isn't clear because a cutoff occurred we may want to sort out which value it has
                if(val == DrawWin || val == DrawLoss)
                {
                    // If the parent node would choose this move anyway, no further evaluation is needed
                    if(redToMove ? val < beta : val > alpha)
                        return createPositionValue(val, 1 + getDepth(posVal));
                }
                else
                    return createPositionValue(val, 1 + getDepth(posVal));
            }
        }

        // If there are exactly 8 pieces on the board the database should have provided us with a value
        // If no value was found in the database, then we're playing a forced move
        // We see it as a Draw
        if(pieceCount == 8)
            return createPositionValue(ValueUnknown, 0);

        // If the board is full, it's a draw
        if(BitBoard::isFull(bitBoard))
            return createPositionValue(Draw, 0);

        // Check if we're not interrupted
        if(keepRunning != 0 && !*keepRunning) return createPositionValue(ValueUnknown, 0);

        // Check if the opponent can win or if we have a forced move
        // Also check which moves would make it possible for the opponent to win directly (and so we don't play them)
        std::vector<int> moves;
        moves.reserve(7);
        const quint64& other = redToMove ? yellowBoard : redBoard;
        for(int col = 0; col < 7; ++col)
        {
            if(!BitBoard::canMove(bitBoard, col)) continue;
            const int row = BitBoard::playableRow(bitBoard, col);

            // Check if the opponent can win on the square above the playable square in this column
            const bool winOnTop = BitBoard::isWinner(other | (Q_UINT64_C(2) << (row + 7 * col)));

            // Check if the opponent can win directly by playing this column
            if(BitBoard::isWinner(other | (Q_UINT64_C(1) << (row + 7 * col))))
            {
                // A double threat can't be stopped
                if(winOnTop)
                    return createPositionValue(redToMove ? Loss : Win, 0);

                // It's a forced move
                moves.clear();
                moves.push_back(col);

                // If another forced move is found, we can't stop the opponent from winning
                while(++col < 7)
                {
                    if(BitBoard::isWinner(other | (Q_UINT64_C(1) << (BitBoard::playableRow(bitBoard, col) + 7 * col))))
                        return createPositionValue(redToMove ? Loss : Win, 0);
                }

                // Stop looking for any other moves
                break;
            }

            // Only play this move if the opponent wouldn't win by it directly
            if(!winOnTop)
                moves.push_back(col);
        }

        // Check if we're not interrupted
        if(keepRunning != 0 && !*keepRunning) return createPositionValue(ValueUnknown, 0);

        // If no moves were found, we lose
        if(moves.empty())
            return createPositionValue(redToMove ? Loss : Win, 0);

        // Find a value for each move
        const unsigned int moveCount = moves.size();
        bool valUnknown = false;
        PositionValue bestScore = redToMove ? Loss : Win;
        quint16 bestDepth = 0;
        for(unsigned int move = 0; move < moveCount; ++move)
        {
            // Check if we're not interrupted
            if(keepRunning != 0 && !*keepRunning) return createPositionValue(ValueUnknown, 0);

            // Dynamically order the moves using the historyHeuristic board
            int row = BitBoard::playableRow(bitBoard, moves[move]);
            int bestHistory = historyHeuristic[redToMove][6 * moves[move] + row];
            unsigned int bestMoveIndex = move;
            for(unsigned int i = move + 1; i < moveCount; ++i)
            {
                const int r = BitBoard::playableRow(bitBoard, moves[i]);
                if(historyHeuristic[redToMove][6 * moves[i] + r] > bestHistory)
                {
                    row = r;
                    bestHistory = historyHeuristic[redToMove][6 * moves[i] + r];
                    bestMoveIndex = i;
                }
            }
            const int bestMoveCol = moves[bestMoveIndex];
            for(; bestMoveIndex > move; --bestMoveIndex)
                moves[bestMoveIndex] = moves[bestMoveIndex - 1];
            moves[move] = bestMoveCol;

            // Make the move
            PositionValue posVal = alphaBeta(BitBoard::move(bitBoard, bestMoveCol, row, redToMove),
                                             redToMove ? redBoard | (Q_UINT64_C(1) << (row + bestMoveCol * 7)) : redBoard,
                                             redToMove ? yellowBoard : yellowBoard | (Q_UINT64_C(1) << (row + bestMoveCol * 7)),
                                             alpha, beta);
            PositionValue val = getValue(posVal);

            // Check if we're not interrupted
            if(keepRunning != 0 && !*keepRunning) return createPositionValue(ValueUnknown, 0);

            if(val == ValueUnknown)
                valUnknown = true;
            else
            {
                // Set the alpha/beta
                if(redToMove)
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
                            --historyHeuristic[redToMove][6 * moves[i] + BitBoard::playableRow(bitBoard, moves[i])];

                        // Reward the good chosen move
                        historyHeuristic[redToMove][6 * moves[move] + BitBoard::playableRow(bitBoard, moves[move])] += move;
                    }

                    // If we do a cutoff at a Draw position it may also be a Win or Loss
                    // But only if not all children have been evaluated yet
                    if(val == Draw && move != moveCount)
                        val = redToMove ? DrawWin : DrawLoss;

                    // Create our result
                    const PositionValue out = createPositionValue(val, 1 + bestDepth);

                    // Only store the position in the database if there are more than 8 pieces on the board
                    // Also only store positions where red is to move (to reduce the database size)
                    // Also only store positions that took a lot of work
                    if(pieceCount > 8 && pieceCount % 3 == 0 && bestDepth > 3)
                    {
                        AlphaBetaSearcher::posDbLockers[pieceCount / 3 - 2]->lockForWrite();
                        AlphaBetaSearcher::posDb[pieceCount / 3 - 2][dbPosition] = out;
                        AlphaBetaSearcher::posDbLockers[pieceCount / 3 - 2]->unlock();
                    }

                    return out;
                }
            }
        }

        // Check if we're not interrupted
        if(keepRunning != 0 && !*keepRunning) return createPositionValue(ValueUnknown, 0);

        // If a ValueUnknown was encountered, the value of this position is unknown
        if(valUnknown)
            return createPositionValue(ValueUnknown, 0);

        // Create our result
        const PositionValue out = createPositionValue(bestScore, 1 + bestDepth);

        // Only store the position in the database if there are more than 8 pieces on the board
        // Also only store positions that took a lot of work
        if(pieceCount > 8 && pieceCount % 3 == 0 && bestDepth > 3)
        {
            AlphaBetaSearcher::posDbLockers[pieceCount / 3 - 2]->lockForWrite();
            AlphaBetaSearcher::posDb[pieceCount / 3 - 2][dbPosition] = out;
            AlphaBetaSearcher::posDbLockers[pieceCount / 3 - 2]->unlock();
        }

        return out;
    }

    /// Static functions:
    void AlphaBetaSearcher::loadPositionDatabase()
    {
        // Acquire a write lock on the the database
        QWriteLocker locker(AlphaBetaSearcher::posDbLockers[0]);

        // Open the file
        QFile file(":/data/positions.db");
        file.open(QIODevice::ReadOnly);

        // Reserve the exact size needed for the database
        AlphaBetaSearcher::posDb[0].reserve(67557);

        // Read the precalculated database
        QByteArray line;
        while(!file.atEnd())
        {
            line = file.readLine();
            if(line.size() != 44) break;

            const char& value = line.at(line.size() - 2);
            const BitBoard bitBoard = BitBoard(BitBoard::board2int(line.left(42).data()));
            const quint64 dbPosition = qMin(bitBoard.toInt(), bitBoard.flip());
            AlphaBetaSearcher::posDb[0][dbPosition] = createPositionValue(value == '2' ? Win : (value == '1' ? Draw : Loss), 0);
        }
    }

    bool AlphaBetaSearcher::positionDatabaseLoaded()
    {
        // Acquire a read lock on the the database
        QReadLocker locker(AlphaBetaSearcher::posDbLockers[0]);

        // Return whether we've loaded the database already
        return !AlphaBetaSearcher::posDb[0].empty();
    }

    AlphaBetaSearcher::PositionValue AlphaBetaSearcher::createPositionValue(const PositionValue& val, const quint16& depth)
    {
        // Lower 3 bits are the value
        // The rest of the bits represent the depth
        return val | (depth << 3);
    }

    AlphaBetaSearcher::PositionValue AlphaBetaSearcher::getValue(const PositionValue& val)
    {
        // 7 is in binary: 111
        // So we only get the lowest 3 bits
        return val & static_cast<quint16>(7);
    }

    quint16 AlphaBetaSearcher::getDepth(const PositionValue& val)
    {
        // Throw away the 3 lowest bits
        return val >> 3;
    }

// Private:
    // Static:
        // Since only the positions with 8, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36 or 39 pieces will be saved we only need 12 QHash maps
        // This is because only positions which are a multiple of 3 are saved, and only if their search depth was greater than 3 (therefore, no positions with 42 pieces will be saved)
        // Also all positions with 8 pieces will be saved (they will be read from the database)
        // The index of each QHash map is calculated as follows: numberOfPieces / 3 - 2
        QHash<quint64, AlphaBetaSearcher::PositionValue> AlphaBetaSearcher::posDb[] =
        {
            QHash<quint64, AlphaBetaSearcher::PositionValue>(), QHash<quint64, AlphaBetaSearcher::PositionValue>(), QHash<quint64, AlphaBetaSearcher::PositionValue>(),
            QHash<quint64, AlphaBetaSearcher::PositionValue>(), QHash<quint64, AlphaBetaSearcher::PositionValue>(), QHash<quint64, AlphaBetaSearcher::PositionValue>(),
            QHash<quint64, AlphaBetaSearcher::PositionValue>(), QHash<quint64, AlphaBetaSearcher::PositionValue>(), QHash<quint64, AlphaBetaSearcher::PositionValue>(),
            QHash<quint64, AlphaBetaSearcher::PositionValue>(), QHash<quint64, AlphaBetaSearcher::PositionValue>(), QHash<quint64, AlphaBetaSearcher::PositionValue>()
        };

        // The locks for knownPositions, every QReadWriteLock in this array represents the lock for using the QHash with the same index
        AlphaBetaSearcher::PositionDatabaseLockers AlphaBetaSearcher::posDbLockers;

    void AlphaBetaSearcher::initHistoryHeuristic()
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

    // PositionDatabaseLockers class
        AlphaBetaSearcher::PositionDatabaseLockers::PositionDatabaseLockers()
        {
            for(int i = 0; i < 12; ++i)
                lockers[i] = new QReadWriteLock();
        }

        AlphaBetaSearcher::PositionDatabaseLockers::~PositionDatabaseLockers()
        {
            for(int i = 0; i < 12; ++i)
                delete lockers[i];
        }

        QReadWriteLock* AlphaBetaSearcher::PositionDatabaseLockers::operator[](const unsigned int& n)
        { return lockers[n]; }
