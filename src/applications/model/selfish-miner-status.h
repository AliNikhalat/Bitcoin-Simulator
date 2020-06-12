#ifndef SELFISH_MINEH_STATUS_H
#define SELFISH_MINEH_STATUS_H

namespace blockchain_attacks{

    typedef struct
    {
        long MinedBlock;
        long SelfishMinerWinBlock;
        long HonestMinerWinBlock;
        int Delta;
    } SelfishMinerStatus;
    
}

#endif