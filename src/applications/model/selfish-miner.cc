#include "ns3/selfish-miner.h"

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
                                ns3::MakeDoubleChecker<double> ())
            .AddAttribute("FixedBlockIntervalGeneration",
                                "The fixed time to wait between two consecutive block generations",
                                ns3::DoubleValue(0),
                                ns3::MakeDoubleAccessor(&SelfishMiner::m_fixedBlockTimeGeneration),
                                ns3::MakeDoubleChecker<double>())

            .AddAttribute ("BlockGenBinSize", 
                                "The block generation bin size",
                                ns3::DoubleValue (-1),
                                ns3::MakeDoubleAccessor (&SelfishMiner::m_blockGenBinSize),
                                ns3::MakeDoubleChecker<double> ())	
            .AddAttribute ("BlockGenParameter", 
                                "The block generation distribution parameter",
                                ns3::DoubleValue (-1),
                                ns3::MakeDoubleAccessor (&SelfishMiner::m_blockGenParameter),
                                ns3::MakeDoubleChecker<double> ());

            return tid;
    }

    SelfishMiner::SelfishMiner() : BitcoinMiner()
    {
        NS_LOG_FUNCTION(this);

        updateTopBlock();
    }

    void SelfishMiner::SetStatus(SelfishMinerStatus* selfishMinerStatus)
    {
        m_selfishMinerStatus = selfishMinerStatus;

        return;
    }

    void SelfishMiner::StartApplication(void)
    {
        std::cout << "Start Selfish Mining" << std::endl;

        if (m_blockGenBinSize < 0 && m_blockGenParameter < 0)
        {
            m_blockGenBinSize = 1. / m_secondsPerMin / 1000;
            m_blockGenParameter = 0.19 * m_blockGenBinSize / 2;
        }
        else
            m_blockGenParameter *= m_hashRate;

        if (m_fixedBlockTimeGeneration == 0)
            m_blockGenTimeDistribution.param(std::geometric_distribution<int>::param_type(m_blockGenParameter));

        if (m_fixedBlockSize > 0)
            m_nextBlockSize = m_fixedBlockSize;
        else
        {
            std::array<double, 201> intervals{0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120, 125,
                                              130, 135, 140, 145, 150, 155, 160, 165, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 220, 225, 230, 235,
                                              240, 245, 250, 255, 260, 265, 270, 275, 280, 285, 290, 295, 300, 305, 310, 315, 320, 325, 330, 335, 340, 345,
                                              350, 355, 360, 365, 370, 375, 380, 385, 390, 395, 400, 405, 410, 415, 420, 425, 430, 435, 440, 445, 450, 455,
                                              460, 465, 470, 475, 480, 485, 490, 495, 500, 505, 510, 515, 520, 525, 530, 535, 540, 545, 550, 555, 560, 565,
                                              570, 575, 580, 585, 590, 595, 600, 605, 610, 615, 620, 625, 630, 635, 640, 645, 650, 655, 660, 665, 670, 675,
                                              680, 685, 690, 695, 700, 705, 710, 715, 720, 725, 730, 735, 740, 745, 750, 755, 760, 765, 770, 775, 780, 785,
                                              790, 795, 800, 805, 810, 815, 820, 825, 830, 835, 840, 845, 850, 855, 860, 865, 870, 875, 880, 885, 890, 895,
                                              900, 905, 910, 915, 920, 925, 930, 935, 940, 945, 950, 955, 960, 965, 970, 975, 980, 985, 990, 995, 1000};
            std::array<double, 200> weights{3.58, 0.33, 0.35, 0.4, 0.38, 0.4, 0.53, 0.46, 0.43, 0.48, 0.56, 0.69, 0.62, 0.62, 0.63, 0.62, 0.62, 0.63, 0.73,
                                            1.96, 0.75, 0.76, 0.73, 0.64, 0.66, 0.66, 0.66, 0.7, 0.66, 0.73, 0.68, 0.66, 0.67, 0.66, 0.72, 0.68, 0.64, 0.61,
                                            0.63, 0.58, 0.66, 0.6, 0.7, 0.62, 0.49, 0.59, 0.58, 0.59, 0.63, 1.59, 0.6, 0.58, 0.54, 0.62, 0.55, 0.54, 0.52,
                                            0.5, 0.53, 0.55, 0.49, 0.47, 0.51, 0.49, 0.52, 0.49, 0.49, 0.49, 0.56, 0.75, 0.51, 0.42, 0.46, 0.47, 0.43, 0.38,
                                            0.39, 0.39, 0.41, 0.43, 0.38, 0.41, 0.36, 0.41, 0.38, 0.42, 0.42, 0.37, 0.41, 0.41, 0.34, 0.32, 0.37, 0.32, 0.34,
                                            0.34, 0.34, 0.32, 0.41, 0.62, 0.33, 0.4, 0.32, 0.32, 0.29, 0.35, 0.32, 0.32, 0.28, 0.26, 0.25, 0.29, 0.26, 0.27,
                                            0.27, 0.24, 0.28, 0.3, 0.27, 0.23, 0.23, 0.28, 0.25, 0.29, 0.24, 0.21, 0.26, 0.29, 0.23, 0.2, 0.24, 0.25, 0.23,
                                            0.21, 0.26, 0.38, 0.24, 0.21, 0.25, 0.23, 0.22, 0.22, 0.24, 0.23, 0.23, 0.26, 0.24, 0.28, 0.64, 9.96, 0.15, 0.11,
                                            0.11, 0.1, 0.1, 0.1, 0.11, 0.11, 0.12, 0.13, 0.12, 0.16, 0.12, 0.13, 0.12, 0.1, 0.13, 0.13, 0.13, 0.25, 0.1, 0.14,
                                            0.14, 0.12, 0.14, 0.14, 0.17, 0.15, 0.19, 0.38, 0.2, 0.19, 0.24, 0.26, 0.36, 1.58, 1.49, 0.1, 0.2, 1.98, 0.05, 0.08,
                                            0.07, 0.07, 0.14, 0.08, 0.08, 0.53, 3.06, 3.31};

            m_blockSizeDistribution = std::piecewise_constant_distribution<double>(intervals.begin(), intervals.end(), weights.begin());
        }

        BitcoinNode::StartApplication();

        m_nodeStats->hashRate = m_hashRate;
        m_nodeStats->miner = 1;

        ns3::BitcoinMiner::ScheduleNextMiningEvent();

        return;
    }

    void SelfishMiner::MineBlock(void)
    {
        std::cout << "selfish number : " << m_hashRate << " mine a block" << std::endl;

        m_selfishMinerStatus->MinedBlock ++;

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

        updateDelta();
        updateTopBlock();

        m_privateChain.push_back(newBlock);

        if(m_selfishMinerStatus->Delta == 0 && GetSelfishChainLength() == 2){
            std::cout << "State 1" << std::endl;

            m_selfishMinerStatus->SelfishMinerWinBlock += 2;

            ReleaseChain(m_privateChain);

            for (const auto &block : m_privateChain)
            {
                ValidateBlock(newBlock);
            }

            resetAttack();
        }

        updateDelta();

        ns3::BitcoinMiner::ScheduleNextMiningEvent();

        return;
    }

    void SelfishMiner::ReleaseChain(std::vector<ns3::Block> blocks)
    {
        std::cout << "Releasing Chain" << std::endl;

        for(const auto& block : blocks){
            m_privateChain.push_back(block);
        }

        rapidjson::Document inv;
        rapidjson::Document block;

        inv.SetObject();
        block.SetObject();

        rapidjson::Value value;
        rapidjson::Value array(rapidjson::kArrayType);
        rapidjson::Value blockInfo(rapidjson::kObjectType);

        value.SetString("block"); //Remove
        inv.AddMember("type", value, inv.GetAllocator());

        if (m_protocolType == ns3::STANDARD_PROTOCOL)
        {
            value = ns3::INV;
            inv.AddMember("message", value, inv.GetAllocator());

            for (auto it = blocks.begin(); it != blocks.end(); it++)
            {
                std::ostringstream stringStream;
                std::string blockHash = stringStream.str();

                stringStream << it->GetBlockHeight() << "/" << it->GetMinerId();
                blockHash = stringStream.str();
                value.SetString(blockHash.c_str(), blockHash.size(), inv.GetAllocator());
                array.PushBack(value, inv.GetAllocator());
            }

            inv.AddMember("inv", array, inv.GetAllocator());
        }
        else if (m_protocolType == ns3::SENDHEADERS)
        {
            value = ns3::HEADERS;
            inv.AddMember("message", value, inv.GetAllocator());

            for (auto it = blocks.begin(); it != blocks.end(); it++)
            {
                value = it->GetBlockHeight();
                blockInfo.AddMember("height", value, inv.GetAllocator());

                value = it->GetMinerId();
                blockInfo.AddMember("minerId", value, inv.GetAllocator());

                value = it->GetParentBlockMinerId();
                blockInfo.AddMember("parentBlockMinerId", value, inv.GetAllocator());

                value = it->GetBlockSizeBytes();
                blockInfo.AddMember("size", value, inv.GetAllocator());

                value = it->GetTimeCreated();
                blockInfo.AddMember("timeCreated", value, inv.GetAllocator());

                value = it->GetTimeReceived();
                blockInfo.AddMember("timeReceived", value, inv.GetAllocator());

                array.PushBack(blockInfo, inv.GetAllocator());
            }

            inv.AddMember("blocks", array, inv.GetAllocator());
        }

        rapidjson::StringBuffer invInfo;
        rapidjson::Writer<rapidjson::StringBuffer> invWriter(invInfo);
        inv.Accept(invWriter);

        rapidjson::StringBuffer blockInfoBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> blockWriter(blockInfoBuffer);
        block.Accept(blockWriter);

        int count = 0;

        for (std::vector<ns3::Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i, ++count)
        {

            const uint8_t delimiter[] = "#";

            m_peersSockets[*i]->Send(reinterpret_cast<const uint8_t *>(invInfo.GetString()), invInfo.GetSize(), 0);
            m_peersSockets[*i]->Send(delimiter, 1, 0);

            if (m_protocolType == ns3::STANDARD_PROTOCOL && !m_blockTorrent)
                m_nodeStats->invSentBytes += m_bitcoinMessageHeader + m_countBytes + inv["inv"].Size() * m_inventorySizeBytes;
            else if (m_protocolType == ns3::SENDHEADERS && !m_blockTorrent)
                m_nodeStats->headersSentBytes += m_bitcoinMessageHeader + m_countBytes + inv["blocks"].Size() * m_headersSizeBytes;
            else if (m_protocolType == ns3::STANDARD_PROTOCOL && m_blockTorrent)
            {
                m_nodeStats->extInvSentBytes += m_bitcoinMessageHeader + m_countBytes + inv["inv"].Size() * m_inventorySizeBytes;
                for (int j = 0; j < inv["inv"].Size(); j++)
                {
                    m_nodeStats->extInvSentBytes += 5; //1Byte(fullBlock) + 4Bytes(numberOfChunks)
                    if (!inv["inv"][j]["fullBlock"].GetBool())
                        m_nodeStats->extInvSentBytes += inv["inv"][j]["availableChunks"].Size() * 1;
                }
            }
            else if (m_protocolType == ns3::SENDHEADERS && m_blockTorrent)
            {
                m_nodeStats->extHeadersSentBytes += m_bitcoinMessageHeader + m_countBytes + inv["blocks"].Size() * m_headersSizeBytes;
                for (int j = 0; j < inv["blocks"].Size(); j++)
                {
                    m_nodeStats->extHeadersSentBytes += 1; //fullBlock
                    if (!inv["blocks"][j]["fullBlock"].GetBool())
                        m_nodeStats->extHeadersSentBytes += inv["inv"][j]["availableChunks"].Size();
                }
            }
        }
    }

    void SelfishMiner::ReceiveBlock(const ns3::Block &newBlock)
    {
        //std::cout << "Receiving a New Block" << newBlock.GetBlockHeight() << std::endl;

        updateDelta();

        std::ostringstream stringStream;
        std::string blockHash = stringStream.str();

        stringStream << newBlock.GetBlockHeight() << "/" << newBlock.GetMinerId();
        blockHash = stringStream.str();

        if (m_blockchain.HasBlock(newBlock) || m_blockchain.IsOrphan(newBlock) || ReceivedButNotValidated(blockHash)){
            NS_LOG_INFO("BitcoinSelfishMiner ReceiveBlock: Bitcoin node " << GetNode()->GetId() << " has already added this block in the m_blockchain: " << newBlock);

            if (m_invTimeouts.find(blockHash) != m_invTimeouts.end())
            {
                m_queueInv.erase(blockHash);
                ns3::Simulator::Cancel(m_invTimeouts[blockHash]);
                m_invTimeouts.erase(blockHash);
            }
        }
        else{
            NS_LOG_INFO("BitcoinSelfishMiner ReceiveBlock: Bitcoin node " << GetNode()->GetId() << " has NOT added this block in the m_blockchain: " << newBlock);

            m_receivedNotValidated[blockHash] = newBlock;

            m_queueInv.erase(blockHash);
            ns3::Simulator::Cancel(m_invTimeouts[blockHash]);
            m_invTimeouts.erase(blockHash);

            m_publicChain.push_back(newBlock);

            if (m_selfishMinerStatus->Delta == 0 && GetSelfishChainLength() == 0)
            {
                std::cout << "State 2" << std::endl;

                m_selfishMinerStatus->HonestMinerWinBlock += 1;

                ValidateBlock(newBlock);

                resetAttack();

                ns3::Simulator::Cancel(m_nextMiningEvent);
                ScheduleNextMiningEvent();
            }
            else if (m_selfishMinerStatus->Delta == 0 && GetSelfishChainLength() == 1)
            {
                std::cout << "State 3" << std::endl;

                
            }
            else if(m_selfishMinerStatus->Delta == 1)
            {
                std::cout << "State 4" << std::endl;

                //! nothing to do...another state define result of this state
            }
            else if(m_selfishMinerStatus->Delta == 2)
            {
                std::cout << "State 5" << std::endl;

                m_selfishMinerStatus->SelfishMinerWinBlock += m_privateChain.size();

                for(const auto& block: m_privateChain){
                    ValidateBlock(newBlock);
                }

                resetAttack();

                ns3::Simulator::Cancel(m_nextMiningEvent);
                ScheduleNextMiningEvent();
            }
            else if(m_selfishMinerStatus->Delta > 2)
            {
                std::cout << "State 6" << std::endl;

                //! nothing to do...another state define result of this state
            }
        }

        updateDelta();

        return;
    }

    void SelfishMiner::StopApplication(void)
    {
        NS_LOG_INFO("Stop Selfish Mining");

        BitcoinNode::StopApplication();
        ns3::Simulator::Cancel(m_nextMiningEvent);

        return;
    }

    void SelfishMiner::DoDispose(void)
    {
        std::cout << "Disposing Selfish miner" << std::endl;

        return;
    }

    void SelfishMiner::updateTopBlock(void)
    {
        NS_LOG_INFO("Updating Top Block");

        if(m_privateChain.size() != 0){
            m_topBlock = m_privateChain[m_privateChain.size() - 1];

            return;
        }

        m_topBlock = *(m_blockchain.GetCurrentTopBlock());

        return;
    }

    void SelfishMiner::updateDelta(void)
    {
        NS_LOG_INFO("updating delta");

        if (m_privateChain.size() > 0 && m_publicChain.size() > 0){

            int delta = m_privateChain[m_privateChain.size() - 1].GetBlockHeight() -
                        m_publicChain[m_publicChain.size() - 1].GetBlockHeight();

            if(delta >= 0){
                m_selfishMinerStatus->Delta = delta;
            }
            else{
                m_selfishMinerStatus->Delta = 0;
            }
        }
        else if (m_privateChain.size() > 0 && m_publicChain.size() == 0){
            m_selfishMinerStatus->Delta = m_privateChain.size();
        }
        else{
            m_selfishMinerStatus->Delta = 0;
        }

        std::cout << "delta is : " << m_selfishMinerStatus->Delta
        << "  selfish size is : " << m_privateChain.size()
        << "  honest size is : " << m_publicChain.size() << std::endl;

        return;
    }

    void SelfishMiner::resetAttack(void)
    {
        m_privateChain.clear();
        m_publicChain.clear();
    }

    int SelfishMiner::GetSelfishChainLength(void)
    {
        return m_privateChain.size();
    }
}