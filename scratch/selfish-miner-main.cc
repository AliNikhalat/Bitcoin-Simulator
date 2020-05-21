#include <iostream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"

uint blockIntervalMinutes;
uint blockNumber = 100;
uint iterations;

NS_LOG_COMPONENT_DEFINE("selfish-miner-main");

void getParametersFromCMD(int argc, char *argv[]);
void printInputStatics();

int main(int argc, char *argv[])
{
    getParametersFromCMD(argc, argv);
    printInputStatics();

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