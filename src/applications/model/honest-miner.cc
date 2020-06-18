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
}