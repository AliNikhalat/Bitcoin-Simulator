#ifndef SELFISH_MINER_H
#define SELFISH_MINER_H

#include <vector>

#include "ns3/bitcoin-miner.h"
#include "ns3/selfish-miner-status.h"

namespace blockchain_attacks 
{
    class SelfishMiner : public ns3::BitcoinMiner
    {
    public:
        static ns3::TypeId GetTypeId(void);

        SelfishMiner();

        void SetStatus(SelfishMinerStatus* selfishMinerStatus);

    protected:
        virtual void StartApplication(void); 

        virtual void MineBlock(void);

        virtual void ReleaseChain(std::vector<ns3::Block> blocks);

        virtual void ReceiveBlock(const ns3::Block &newBlock);

        virtual void StopApplication(void);

        virtual void DoDispose(void);

    private:
        std::vector<ns3::Block> m_privateChain;
        std::vector<ns3::Block> m_publicChain;
        ns3::Block m_topBlock;

        SelfishMinerStatus* m_selfishMinerStatus;

        void updateTopBlock(void);
        void updateDelta(void);

        int GetSelfishChainLength(void);
    };
};

#endif