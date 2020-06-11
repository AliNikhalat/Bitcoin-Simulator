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

        std::cout << "running constructor" << std::endl;
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

        return;
    }

    void SelfishMiner::MineBlock(void)
    {
        std::cout << "Mine a Selfish Block" << std::endl;

        return;
    }

    void SelfishMiner::DoDispose(void)
    {
        std::cout << "Disposing Selfish miner" << std::endl;
        
        return;
    }
}