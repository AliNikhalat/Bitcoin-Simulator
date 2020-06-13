#include <iostream>
#include <time.h>
#include <sys/time.h>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"

#include "ns3/bitcoin.h"

#include "ns3/selfish-miner-status.h"

uint blockIntervalMinutes;
uint blockNumber = 1;
uint iterations = 1;

NS_LOG_COMPONENT_DEFINE("selfish-miner-main");

void getParametersFromCMD(int argc, char *argv[]);
void printInputStatics();
void printSelfishAttackStatus(blockchain_attacks::SelfishMinerStatus selfishMinerStatus);
double get_wall_time();

using namespace ns3;

int main(int argc, char *argv[])
{
    getParametersFromCMD(argc, argv);
    printInputStatics();

    const int secsPerMin = 60;
    const double realAverageBlockGenIntervalMinutes = 10; //minutes
    const uint16_t bitcoinPort = 8333;

    double minersHash[] = {0.185, 0.159, 0.133, 0.066, 0.054,
                           0.029, 0.016, 0.012, 0.012, 0.012, 0.009,
                           0.005, 0.005, 0.002, 0.002, 0.3};

    enum BitcoinRegion minersRegions[] = {ASIA_PACIFIC, ASIA_PACIFIC, NORTH_AMERICA, ASIA_PACIFIC, NORTH_AMERICA,
                                          EUROPE, EUROPE, NORTH_AMERICA, NORTH_AMERICA, NORTH_AMERICA, EUROPE,
                                          NORTH_AMERICA, NORTH_AMERICA, NORTH_AMERICA, NORTH_AMERICA, ASIA_PACIFIC};

    int totalNoNodes = sizeof(minersHash) / sizeof(double);
    int noMiners = totalNoNodes;
    int attackerId = totalNoNodes - 1;
    int minConnectionsPerNode = 1;
    int maxConnectionsPerNode = 1;

    double averageBlockGenIntervalSeconds = 10 * secsPerMin; //seconds
    double averageBlockGenIntervalMinutes = averageBlockGenIntervalSeconds / secsPerMin;

    int start = 0;
    double stop = blockNumber * averageBlockGenIntervalMinutes;

    double tSimStart = 0;
    double tSimFinish = 0;

    long blockSize = 450000 * averageBlockGenIntervalMinutes / realAverageBlockGenIntervalMinutes;

    nodeStatistics* nodeStatic = new nodeStatistics[totalNoNodes];
    auto selfishStatus = blockchain_attacks::SelfishMinerStatus();

    selfishStatus.Delta = 0;
    selfishStatus.HonestMinerWinBlock = 0;
    selfishStatus.SelfishMinerWinBlock = 0;
    selfishStatus.MinedBlock = 0;

    srand(1000);
    Time::SetResolution(Time::NS);

    for(int i{0}; i < iterations; i++){
        std::cout << "iteration number : " << i << std::endl;

        Ipv4InterfaceContainer ipv4InterfaceContainer;
        std::map<uint32_t, std::vector<Ipv4Address>> nodesConnections;
        std::map<uint32_t, std::map<Ipv4Address, double>> peersDownloadSpeeds;
        std::map<uint32_t, std::map<Ipv4Address, double>> peersUploadSpeeds;
        std::map<uint32_t, nodeInternetSpeeds> nodesInternetSpeeds;
        std::vector<uint32_t> miners;

        BitcoinTopologyHelper bitcoinTopologyHelper(1, totalNoNodes, noMiners, minersRegions,
                                                    Cryptocurrency::BITCOIN, minConnectionsPerNode,
                                                    maxConnectionsPerNode, 2, 0);

        InternetStackHelper stack;
        bitcoinTopologyHelper.InstallStack(stack);

        bitcoinTopologyHelper.AssignIpv4Addresses(Ipv4AddressHelperCustom("1.0.0.0", "255.255.255.0", false));
        ipv4InterfaceContainer = bitcoinTopologyHelper.GetIpv4InterfaceContainer();
        nodesConnections = bitcoinTopologyHelper.GetNodesConnectionsIps();
        miners = bitcoinTopologyHelper.GetMiners();
        peersDownloadSpeeds = bitcoinTopologyHelper.GetPeersDownloadSpeeds();
        peersUploadSpeeds = bitcoinTopologyHelper.GetPeersUploadSpeeds();
        nodesInternetSpeeds = bitcoinTopologyHelper.GetNodesInternetSpeeds();

        ApplicationContainer bitcoinMiners;

        for(size_t i{0}; i < noMiners; i++){
            auto miner = miners[i];
            Ptr<Node> targetNode = bitcoinTopologyHelper.GetNode(miner);

            BitcoinMinerHelper bitcoinMinerHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), bitcoinPort),
                                                  nodesConnections[miner], noMiners, peersDownloadSpeeds[miner], peersUploadSpeeds[miner],
                                                  nodesInternetSpeeds[miner], nodeStatic, minersHash[miner], averageBlockGenIntervalSeconds);

            if(attackerId == miner){
                std::cout << "attacker id is : " << attackerId << std::endl;
                bitcoinMinerHelper.SetMinerType(MY_SELFISH_MINER);
                bitcoinMinerHelper.SetSelfishStatus(selfishStatus);
            }

            bitcoinMinerHelper.SetPeersAddresses(nodesConnections[miner]);
            bitcoinMinerHelper.SetPeersDownloadSpeeds(peersDownloadSpeeds[miner]);
            bitcoinMinerHelper.SetPeersUploadSpeeds(peersUploadSpeeds[miner]);
            bitcoinMinerHelper.SetNodeInternetSpeeds(nodesInternetSpeeds[miner]);
            bitcoinMinerHelper.SetNodeStats(&nodeStatic[miner]);

            bitcoinMiners.Add(bitcoinMinerHelper.Install(targetNode));
        }

        bitcoinMiners.Start(Seconds(start));
        bitcoinMiners.Stop(Minutes(stop));

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();

        Simulator::Stop(Minutes(stop + 0.1));

        tSimStart = get_wall_time();
        Simulator::Run();
        Simulator::Destroy();
        tSimFinish = get_wall_time();

        printSelfishAttackStatus(selfishStatus);
    }

    return 0;
}

void getParametersFromCMD(int argc, char* argv[])
{
    ns3::CommandLine cmd;

    cmd.AddValue("blockNumber", "number of blocks", blockNumber);
    cmd.AddValue("blockInterValMinutes", "interval time of mining block", blockIntervalMinutes);
    cmd.AddValue("iterations", "number of iterations for running algorithm", iterations);

    cmd.Parse(argc, argv);
}

void printInputStatics()
{
    std::cout << "number of blocks is : " << blockNumber << std::endl;
    std::cout << "block interval in minutes is : " << blockIntervalMinutes << std::endl;
    std::cout << "number of iteration is : " << iterations << std::endl;
}

double get_wall_time()
{
    struct timeval time;
    if (gettimeofday(&time, NULL))
    {
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void printSelfishAttackStatus(blockchain_attacks::SelfishMinerStatus selfishMinerStatus)
{
    std::cout << "*****************Selfish Attack Status*********************" << std::endl;
    std::cout << "Mined Block Number : " << selfishMinerStatus.MinedBlock << std::endl;
    std::cout << "Honest Miner Win Block : " << selfishMinerStatus.HonestMinerWinBlock << std::endl;
    std::cout << "Selfish Miner Win Block : " << selfishMinerStatus.SelfishMinerWinBlock << std::endl;

    return;
}