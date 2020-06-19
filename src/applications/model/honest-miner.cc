#include "ns3/honest-miner.h"

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

    NS_LOG_COMPONENT_DEFINE("honest-miner");

    NS_OBJECT_ENSURE_REGISTERED(HonestMiner);

    ns3::TypeId HonestMiner::GetTypeId(void)
    {
        static ns3::TypeId tid = ns3::TypeId("blockchain_attacks::HonestMiner")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<HonestMiner>()
            .AddAttribute("Local",
                            "The Address on which to Bind the rx socket.",
                            ns3::AddressValue(),
                            ns3::MakeAddressAccessor(&HonestMiner::m_local),
                            ns3::MakeAddressChecker())
            .AddAttribute("Protocol",
                            "The type id of the protocol to use for the rx socket.",
                            ns3::TypeIdValue(ns3::UdpSocketFactory::GetTypeId()),
                            ns3::MakeTypeIdAccessor(&HonestMiner::m_tid),
                            ns3::MakeTypeIdChecker())
            .AddAttribute("BlockTorrent",
                            "Enable the BlockTorrent protocol",
                            ns3::BooleanValue(false),
                            ns3::MakeBooleanAccessor(&HonestMiner::m_blockTorrent),
                            ns3::MakeBooleanChecker())
            .AddAttribute("SPV",
                            "Enable SPV Mechanism",
                            ns3::BooleanValue(false),
                            ns3::MakeBooleanAccessor(&HonestMiner::m_spv),
                            ns3::MakeBooleanChecker())
            .AddAttribute("NumberOfMiners",
                            "The number of miners",
                            ns3::UintegerValue(16),
                            ns3::MakeUintegerAccessor(&HonestMiner::m_noMiners),
                            ns3::MakeUintegerChecker<uint32_t>())
            .AddAttribute("FixedBlockSize",
                            "The fixed size of the block",
                            ns3::UintegerValue(0),
                            ns3::MakeUintegerAccessor(&HonestMiner::m_fixedBlockSize),
                            ns3::MakeUintegerChecker<uint32_t>())
            .AddAttribute("FixedBlockIntervalGeneration",
                            "The fixed time to wait between two consecutive block generations",
                            ns3::DoubleValue(0),
                            ns3::MakeDoubleAccessor(&HonestMiner::m_fixedBlockTimeGeneration),
                            ns3::MakeDoubleChecker<double>())
            .AddAttribute("InvTimeoutMinutes",
                            "The timeout of inv messages in minutes",
                            ns3::TimeValue(ns3::Minutes(20)),
                            ns3::MakeTimeAccessor(&HonestMiner::m_invTimeoutMinutes),
                            ns3::MakeTimeChecker())
            .AddAttribute("HashRate",
                            "The hash rate of the miner",
                            ns3::DoubleValue(0.2),
                            ns3::MakeDoubleAccessor(&HonestMiner::m_hashRate),
                            ns3::MakeDoubleChecker<double>())
            .AddAttribute("BlockGenBinSize",
                            "The block generation bin size",
                            ns3::DoubleValue(-1),
                            ns3::MakeDoubleAccessor(&HonestMiner::m_blockGenBinSize),
                            ns3::MakeDoubleChecker<double>())
            .AddAttribute("BlockGenParameter",
                            "The block generation distribution parameter",
                            ns3::DoubleValue(-1),
                            ns3::MakeDoubleAccessor(&HonestMiner::m_blockGenParameter),
                            ns3::MakeDoubleChecker<double>())
            .AddAttribute("AverageBlockGenIntervalSeconds",
                            "The average block generation interval we aim at (in seconds)",
                            ns3::DoubleValue(10 * 60),
                            ns3::MakeDoubleAccessor(&HonestMiner::m_averageBlockGenIntervalSeconds),
                            ns3::MakeDoubleChecker<double>())
            .AddAttribute("Cryptocurrency",
                            "BITCOIN, LITECOIN, DOGECOIN",
                            ns3::UintegerValue(0),
                            ns3::MakeUintegerAccessor(&HonestMiner::m_cryptocurrency),
                            ns3::MakeUintegerChecker<uint32_t>())
            .AddAttribute("ChunkSize",
                            "The fixed size of the block chunk",
                            ns3::UintegerValue(100000),
                            ns3::MakeUintegerAccessor(&HonestMiner::m_chunkSize),
                            ns3::MakeUintegerChecker<uint32_t>())
            .AddTraceSource("Rx",
                            "A packet has been received",
                            ns3::MakeTraceSourceAccessor(&HonestMiner::m_rxTrace),
                            "ns3::Packet::AddressTracedCallback");
        return tid;
    }

    HonestMiner::HonestMiner() : ns3::BitcoinMiner()
    {
        NS_LOG_FUNCTION(this);
    }

    void HonestMiner::SetStatus(SelfishMinerStatus *selfishMinerStatus)
    {
        m_selfishMinerStatus = selfishMinerStatus;

        return;
    }

    void HonestMiner::SetGamma(double gamma)
    {
        m_gamma = gamma;

        return;
    }

    void HonestMiner::MineBlock(void)
    {
        std::cout << "honest number : " << m_hashRate << " mine a block" << std::endl;

        m_selfishMinerStatus->MinedBlock++;

        int height = m_blockchain.GetCurrentTopBlock()->GetBlockHeight() + 1;
        int minerId = GetNode()->GetId();
        int parentBlockMinerId = m_blockchain.GetCurrentTopBlock()->GetMinerId();
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

        rapidjson::Document inv;
        rapidjson::Document block;

        inv.SetObject();
        block.SetObject();

        rapidjson::Value value;
        rapidjson::Value array(rapidjson::kArrayType);
        rapidjson::Value blockInfo(rapidjson::kObjectType);

        value.SetString("block");
        inv.AddMember("type", value, inv.GetAllocator());

        if (m_protocolType == ns3::STANDARD_PROTOCOL)
        {
            if (!m_blockTorrent)
            {
                value = ns3::INV;
                inv.AddMember("message", value, inv.GetAllocator());

                value.SetString(blockHash.c_str(), blockHash.size(), inv.GetAllocator());
                array.PushBack(value, inv.GetAllocator());

                inv.AddMember("inv", array, inv.GetAllocator());
            }
            else
            {
                value = ns3::EXT_INV;
                inv.AddMember("message", value, inv.GetAllocator());

                value.SetString(blockHash.c_str(), blockHash.size(), inv.GetAllocator());
                blockInfo.AddMember("hash", value, inv.GetAllocator());

                value = newBlock.GetBlockSizeBytes();
                blockInfo.AddMember("size", value, inv.GetAllocator());

                value = true;
                blockInfo.AddMember("fullBlock", value, inv.GetAllocator());

                array.PushBack(blockInfo, inv.GetAllocator());
                inv.AddMember("inv", array, inv.GetAllocator());
            }
        }
        else if (m_protocolType == ns3::SENDHEADERS)
        {

            value = newBlock.GetBlockHeight();
            blockInfo.AddMember("height", value, inv.GetAllocator());

            value = newBlock.GetMinerId();
            blockInfo.AddMember("minerId", value, inv.GetAllocator());

            value = newBlock.GetParentBlockMinerId();
            blockInfo.AddMember("parentBlockMinerId", value, inv.GetAllocator());

            value = newBlock.GetBlockSizeBytes();
            blockInfo.AddMember("size", value, inv.GetAllocator());

            value = newBlock.GetTimeCreated();
            blockInfo.AddMember("timeCreated", value, inv.GetAllocator());

            value = newBlock.GetTimeReceived();
            blockInfo.AddMember("timeReceived", value, inv.GetAllocator());

            if (!m_blockTorrent)
            {
                value = ns3::HEADERS;
                inv.AddMember("message", value, inv.GetAllocator());
            }
            else
            {
                value = ns3::EXT_HEADERS;
                inv.AddMember("message", value, inv.GetAllocator());

                value = true;
                blockInfo.AddMember("fullBlock", value, inv.GetAllocator());
            }

            array.PushBack(blockInfo, inv.GetAllocator());
            inv.AddMember("blocks", array, inv.GetAllocator());
        }

        m_meanBlockReceiveTime = (m_blockchain.GetTotalBlocks() - 1) / static_cast<double>(m_blockchain.GetTotalBlocks()) * m_meanBlockReceiveTime + (currentTime - m_previousBlockReceiveTime) / (m_blockchain.GetTotalBlocks());
        m_previousBlockReceiveTime = currentTime;

        m_meanBlockPropagationTime = (m_blockchain.GetTotalBlocks() - 1) / static_cast<double>(m_blockchain.GetTotalBlocks()) * m_meanBlockPropagationTime;

        m_meanBlockSize = (m_blockchain.GetTotalBlocks() - 1) / static_cast<double>(m_blockchain.GetTotalBlocks()) * m_meanBlockSize + (m_nextBlockSize) / static_cast<double>(m_blockchain.GetTotalBlocks());

        m_blockchain.AddBlock(newBlock);

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

        m_minerAverageBlockGenInterval = m_minerGeneratedBlocks / static_cast<double>(m_minerGeneratedBlocks + 1) * m_minerAverageBlockGenInterval + (ns3::Simulator::Now().GetSeconds() - m_previousBlockGenerationTime) / (m_minerGeneratedBlocks + 1);
        m_minerAverageBlockSize = m_minerGeneratedBlocks / static_cast<double>(m_minerGeneratedBlocks + 1) * m_minerAverageBlockSize + static_cast<double>(m_nextBlockSize) / (m_minerGeneratedBlocks + 1);
        m_previousBlockGenerationTime = ns3::Simulator::Now().GetSeconds();
        m_minerGeneratedBlocks++;

        ScheduleNextMiningEvent();
    }
}