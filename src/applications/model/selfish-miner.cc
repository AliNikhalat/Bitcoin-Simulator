#include "selfish-miner.h"

#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/bitcoin-selfish-miner.h"

#include "../../rapidjson/document.h"
#include "../../rapidjson/writer.h"
#include "../../rapidjson/stringbuffer.h"

namespace blockchain_attacks{

    NS_LOG_COMPONENT_DEFINE("selfish-miner");

    NS_OBJECT_ENSURE_REGISTERED(SelfishMiner);

    ns3::TypeId SelfishMiner::GetTypeId()
    {
        static ns3::TypeId tid = ns3::TypeId("blockchain_attacks::SelfishMiner")
            .SetParent<ns3::Application>()
            .SetGroupName("Applications")
            .AddConstructor<SelfishMiner>()
            .AddAttribute("Local",
                            "The Address on which to Bind the rx socket.",
                            ns3::AddressValue(),
                            ns3::MakeAddressAccessor(&SelfishMiner::m_local),
                            ns3::MakeAddressChecker())
            .AddAttribute ("Protocol",
                            "The type id of the protocol to use for the rx socket.",
                            ns3::TypeIdValue (ns3::UdpSocketFactory::GetTypeId ()),
                            ns3::MakeTypeIdAccessor (&SelfishMiner::m_tid),
                            ns3::MakeTypeIdChecker ())
            .AddTraceSource ("Rx",
                                "A packet has been received",
                                ns3::MakeTraceSourceAccessor (&SelfishMiner::m_rxTrace),
                                "ns3::Packet::AddressTracedCallback")
            .AddAttribute("NumberOfMiners",
                                "The number of miners",
                                ns3::UintegerValue(16),
                                MakeUintegerAccessor(&SelfishMiner::m_noMiners),
                                ns3::MakeUintegerChecker<uint32_t>())
            .AddAttribute ("HashRate", 
                                "The hash rate of the selfish miner",
                                ns3::DoubleValue (0.2),
                                ns3::MakeDoubleAccessor (&SelfishMiner::m_hashRate),
                                ns3::MakeDoubleChecker<double> ())
            .AddAttribute ("AverageBlockGenIntervalSeconds", 
                                "The average block generation interval we aim at (in seconds)",
                                ns3::DoubleValue (10*60),
                                ns3::MakeDoubleAccessor (&SelfishMiner::m_averageBlockGenIntervalSeconds),
                                ns3::MakeDoubleChecker<double> ());	

            return tid;
    }

    SelfishMiner::SelfishMiner()
    {
        NS_LOG_FUNCTION(this);
        
        updateTopBlock();
    }

    void SelfishMiner::StartApplication(void)
    {
        std::cout << "Start Selfish Mining" << std::endl;

        m_nodeStats->hashRate = m_hashRate;
        m_nodeStats->miner = 1;

        ScheduleNextMiningEvent();

        return;
    }

    void SelfishMiner::StopApplication(void)
    {
        std::cout << "Stop Selfish Mining" << std::endl;

        BitcoinNode::StopApplication();
        ns3::Simulator::Cancel(m_nextMiningEvent);

        return;
    }

    void SelfishMiner::MineBlock(void)
    {
        std::cout << "Mine a Selfish Block" << std::endl;

        int height = m_topBlock.GetBlockHeight() + 1;
        int minerId = GetNode()->GetId();
        int parentBlockMinerId = m_topBlock.GetMinerId();
        double currentTime = ns3::Simulator::Now().GetSeconds();

        std::ostringstream stringStream;
        std::string blockHash;
        stringStream << height << "/" << minerId;
        blockHash = stringStream.str();

        if (m_fixedBlockSize > 0)
            m_nextBlockSize = m_fixedBlockSize;
        else
        {
            m_nextBlockSize = m_blockSizeDistribution(m_generator) * 1000; 

            if (m_cryptocurrency == ns3::BITCOIN)
            {
                // The block size is linearly dependent on the averageBlockGenIntervalSeconds
                if (m_nextBlockSize < m_maxBlockSize - m_headersSizeBytes)
                    m_nextBlockSize = m_nextBlockSize * m_averageBlockGenIntervalSeconds / m_realAverageBlockGenIntervalSeconds + m_headersSizeBytes;
                else
                    m_nextBlockSize = m_nextBlockSize * m_averageBlockGenIntervalSeconds / m_realAverageBlockGenIntervalSeconds;
            }
        }

        if (m_nextBlockSize < m_averageTransactionSize)
            m_nextBlockSize = m_averageTransactionSize + m_headersSizeBytes;

        ns3::Block newBlock(height, minerId, parentBlockMinerId, m_nextBlockSize,
                       currentTime, currentTime, ns3::Ipv4Address("127.0.0.1"));

        m_privateChain.push_back(newBlock);
        updateTopBlock();

        

        return;
    }

    void SelfishMiner::DoDispose(void)
    {
        std::cout << "Disposing Selfish miner" << std::endl;
        
        return;
    }

    void SelfishMiner::updateTopBlock(void)
    {
        std::cout << "Updating Top Block" << std::endl;

        if(m_privateChain.size() == 0){
            m_topBlock = *(m_blockchain.GetCurrentTopBlock());

            return;
        }

        m_topBlock = m_privateChain[m_privateChain.size() - 1];

        return;
    }
}