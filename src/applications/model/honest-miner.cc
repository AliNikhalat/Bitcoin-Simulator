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