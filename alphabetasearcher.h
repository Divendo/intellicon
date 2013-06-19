#ifndef ALPHABETASEARCHER_H
#define ALPHABETASEARCHER_H

#include <QObject>
#include <QRunnable>
#include <QReadWriteLock>
#include "bitboard.h"

class AlphaBetaSearcher : public QObject, public QRunnable
{
    Q_OBJECT

    public:
        AlphaBetaSearcher(const BitBoard& board, const int& move);

        /// PositionValue:
        //  The first 3 bits in the unsigned int tell us the value of that position
        //  The remaining bits tell us how deep we went (from that position on) to determine that value
        typedef quint16 PositionValue;

        // The possible game theoretical values
        static const PositionValue ValueUnknown;
        static const PositionValue Loss;
        static const PositionValue DrawLoss;
        static const PositionValue Draw;
        static const PositionValue DrawWin;
        static const PositionValue Win;

        // Sets a pointer to a boolean that becomes false when this thread is interrupted
        void setInterruptedPointer(const bool* p);

        // Called if this class is used as QRunnable
        // This call alphaBeta() with the board that's given in the constructor
        // The result is outputted through the done() signal
        void run();

        // Finds the value of the given position
        PositionValue alphaBeta(const quint64& bitBoard, const quint64& redBoard, const quint64& yellowBoard, PositionValue alpha, PositionValue beta);

        // Load the known position from the database
        static void loadPositionDatabase();
        // Whether or not the position database is loaded
        static bool positionDatabaseLoaded();

        // Creates a PositionValue
        static PositionValue createPositionValue(const PositionValue& val, const quint16& depth);
        // Get the value part of a PositionValue
        static PositionValue getValue(const PositionValue& val);
        // Get how deep we went for this PositionValue
        static quint16 getDepth(const PositionValue& val);

    signals:
        void done(const int& move, const quint16& val);
        
    private:
        BitBoard board;                 // The board to use when run() is called
        int move;                       // The move that was given in the constructor, this will be outputted with the result through the done() signal
        const bool* keepRunning;        // Whether we should keep searching for moves (true) or are interrupted (false)

        // A simple class used to manage the position database lockers
        class PositionDatabaseLockers
        {
            public:
                PositionDatabaseLockers();
                ~PositionDatabaseLockers();

                QReadWriteLock* operator[](const unsigned int& n);

            private:
                QReadWriteLock* lockers[12];
        };

        // Positions of which the value is known
        static QHash<quint64, PositionValue> posDb[];
        // Lockers used to lock the position database
        static PositionDatabaseLockers posDbLockers;

        // Used for the history heuristic optimization
        int historyHeuristic[2][42];
        // Initialise the history heuristic array
        void initHistoryHeuristic();
};

#endif // ALPHABETASEARCHER_H
