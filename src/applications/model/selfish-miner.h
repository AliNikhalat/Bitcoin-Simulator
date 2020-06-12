#ifndef SELFISH_MINER_H
#define SELFISH_MINER_H

#include <vector>

#include "bitcoin-miner.h"
#include "selfish-miner-status.h"

namespace blockchain_attacks 
{
    class ns3::Address;
    class ns3::Socket;
    class ns3::Packet;

    class SelfishMiner : public ns3::BitcoinMiner
    {
    public:
        static ns3::TypeId GetTypeId(void);

        SelfishMiner(SelfishMinerStatus* selfishMinerStatus);

    protected:
        virtual void StartApplication(void); 
        virtual void StopApplication(void);

        virtual void MineBlock(void);

        virtual void DoDispose(void);

    private:
        std::vector<ns3::Block> m_privateChain;
        ns3::Block m_topBlock;
        ns3::Block m_selfishTopBlock;
        ns3::Block m_honestTopBlock;

        SelfishMinerStatus* m_selfishMinerStatus;

        void updateTopBlock(void);
        void updateDelta(void);
    };
};

#endif