#ifndef HONEST_MINER_H
#define HONEST_MINER_H

#include "ns3/bitcoin-miner.h"
#include "ns3/selfish-miner-status.h"

namespace blockchain_attacks {

    class HonestMiner : public ns3::BitcoinMiner
    {
    public:
        static ns3::TypeId GetTypeId(void);
        
        HonestMiner();

        void SetStatus(SelfishMinerStatus *selfishMinerStatus);

        void SetGamma(double gamma);
    
    protected:
        virtual void MineBlock(void);

    private:
        double m_gamma;

        SelfishMinerStatus *m_selfishMinerStatus;

        double generateRandomGamma(void);
    };

}

#endif