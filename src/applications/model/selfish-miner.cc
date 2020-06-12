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
                                ns3::MakeDoubleChecker<double> ());	

            return tid;
    }

    SelfishMiner::SelfishMiner()
    {
        NS_LOG_FUNCTION(this);

        updateTopBlock();
    }

    void SelfishMiner::SetStatus(SelfishMinerStatus selfishMinerStatus)
    {
        m_selfishMinerStatus = selfishMinerStatus;

        return;
    }

    void SelfishMiner::StartApplication(void)
    {
        std::cout << "Start Selfish Mining" << std::endl;

        BitcoinNode::StartApplication();

        m_nodeStats->hashRate = m_hashRate;
        m_nodeStats->miner = 1;

        ScheduleNextMiningEvent();

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

        updateDelta();
        updateTopBlock();

        if(m_selfishMinerStatus.Delta == 0 && GetSelfishChainLength() == 2){
            m_selfishMinerStatus.SelfishMinerWinBlock += 2;
            ReleaseChain(m_privateChain);
            updateDelta();
        }        

        return;
    }

    void SelfishMiner::ReleaseChain(std::vector<ns3::Block> blocks)
    {
        std::cout << "Releasing Chain" << std::endl;

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
        std::cout << "Receiving a New Block" << std::endl;

        return;
    }

    void SelfishMiner::StopApplication(void)
    {
        std::cout << "Stop Selfish Mining" << std::endl;
        std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        BitcoinNode::StopApplication();
        ns3::Simulator::Cancel(m_nextMiningEvent);
        std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!8" << std::endl;

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

        // if(m_blockchain.GetBlockchainHeight() == 0){
        //     return;
        // }

        // m_honestTopBlock = *(m_blockchain.GetCurrentTopBlock());

        // if(m_privateChain.size() == 0){
        //     m_topBlock = m_honestTopBlock;
            
        //     return;
        // }

        // m_selfishTopBlock = m_privateChain[m_privateChain.size() - 1];
        // m_topBlock = m_selfishTopBlock;

        std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;

        return;
    }

    void SelfishMiner::updateDelta(void)
    {
        std::cout << "updating delta" << std::endl;

        // if(m_privateChain.size() != 0)
        // {
        //     int delta = m_selfishTopBlock.GetBlockHeight() - m_honestTopBlock.GetBlockHeight();
        //     if(delta >= 0){
        //         m_selfishMinerStatus->Delta = m_selfishTopBlock.GetBlockHeight() - m_honestTopBlock.GetBlockHeight();
        //     }
        //     else{
        //         m_selfishMinerStatus->Delta = 0;
        //     }

        //     return;
        // }

        // m_selfishMinerStatus->Delta = 0;

        std::cout << "**************************" << std::endl;

        return;
    }

    int SelfishMiner::GetSelfishChainLength(void)
    {
        return m_privateChain.size();
    }
}