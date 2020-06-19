#ifndef SELFISH_MINEH_STATUS_H
#define SELFISH_MINEH_STATUS_H

namespace blockchain_attacks{

    typedef struct
    {
        int MinedBlock;

        int SelfishMinerWinBlock;
        int HonestMinerWinBlock;

        int SelfishChainLength;
        int HonestChainLength;

        int Delta;
    } SelfishMinerStatus;
    
}

#endif