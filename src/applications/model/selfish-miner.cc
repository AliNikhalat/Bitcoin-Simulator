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
                                "ns3::Packet::AddressTracedCallback");

        return tid;
    }
}