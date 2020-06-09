#include <iostream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"

#include "ns3/bitcoin.h"

uint blockIntervalMinutes;
uint blockNumber = 100;
uint iterations = 1;

NS_LOG_COMPONENT_DEFINE("selfish-miner-main");

void getParametersFromCMD(int argc, char *argv[]);
void printInputStatics();

using namespace ns3;

int main(int argc, char *argv[])
{
    getParametersFromCMD(argc, argv);
    printInputStatics();

    const int secsPerMin = 60;
    const double realAverageBlockGenIntervalMinutes = 10; //minutes

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

    long blockSize = 450000 * averageBlockGenIntervalMinutes / realAverageBlockGenIntervalMinutes;

    nodeStatistics* nodeStatic = new nodeStatistics[totalNoNodes];

    srand(1000);
    Time::SetResolution(Time::NS);

    for(int i{0}; i < iterations; i++){
        std::cout << "iteration number : " << i << std::endl;
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