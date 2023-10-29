#include "ns3/ssid.h"
#include "ns3/wifi-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/nix-vector-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-phy.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/csma-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/nstime.h"
#include "ns3/dsr-module.h"

#include <iostream>
#include <vector>
#include <string.h>

using namespace ns3;

int PACKET_SIZE_KB = 1;
double txpDistance = 5.0;
bool verbose = false;
double totalRx;
double totalTx;
Time lastTime;
Time initialTime;
bool isInit;
std::string bottleNeckLinkBw = "5Mbps";
std::string bottleNeckLinkDelay = "50ms";
uint16_t port = 5001;
std::ofstream thp_out;
std::ofstream dr_out;

void calMetrix(int num, std::string exp_name)
{
    std::ofstream ofs_thp ("scratch/1905113/mobile/"+exp_name+"/throughput/"+exp_name+"-"+std::to_string(num)+".dat", std::ios::out | std::ios::app);
    std::ofstream ofs_deliver ("scratch/1905113/mobile/"+exp_name+"/delivery_ratio/"+exp_name+"-"+std::to_string(num)+".dat", std::ios::out | std::ios::app);

    
    //  Output Measurement Varibales
    Time curTime = Now();

    double throughput = totalRx / (lastTime.GetSeconds()-initialTime.GetSeconds())/1000;
    double deliveryRatio = (totalRx * 100.00)/totalTx;

    ofs_thp<<curTime.GetSeconds()<<" "<<throughput<<std::endl;
    ofs_deliver<<curTime.GetSeconds()<<" "<<deliveryRatio<<std::endl;
    Simulator::Schedule (Seconds (0.25), &calMetrix, num,exp_name);

}

static void CalculateTotalRX(Ptr<const Packet>packet, const Address &address)
{
    totalRx += packet->GetSize()*8;
    if(isInit){
        initialTime = Now();
        isInit=false;
    }
    lastTime = Now();
}

static void CalculateTotalTX(Ptr<const Packet>packet)
{
    totalTx += packet->GetSize()*8;
}

void topology(int nNode, int nFlow,int nPPS,int speed, std::string exp_name)
{   
    std::cout<<"Running for variable "<< exp_name<<": node: "<<2*nNode+2<<", Flow: "<<nFlow<<", Packet per second: "<<nPPS<<", Speed: "<<speed<<std::endl;

    if(verbose)
    {
        LogComponentEnable("PacketSink",LOG_LEVEL_INFO);
    }
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1024));
    NodeContainer p2pBottleNeckNodes;
    p2pBottleNeckNodes.Create (2);

    // Create the point-to-point link helpers
    PointToPointHelper bottleNeckLink;
    bottleNeckLink.SetDeviceAttribute  ("DataRate", StringValue (bottleNeckLinkBw));
    bottleNeckLink.SetChannelAttribute ("Delay", StringValue (bottleNeckLinkDelay));

    NetDeviceContainer p2pBottleNeckDevices;
    p2pBottleNeckDevices = bottleNeckLink.Install (p2pBottleNeckNodes);

    //wifi left + right
    NodeContainer wifiStaNodesLeft,wifiStaNodesRight;
    wifiStaNodesLeft.Create (nNode);
    wifiStaNodesRight.Create (nNode);
    NodeContainer wifiApNodeLeft = p2pBottleNeckNodes.Get (0);
    NodeContainer wifiApNodeRight = p2pBottleNeckNodes.Get (1);

    // constructs the wifi devices and the interconnection channel between these wifi nodes.
    YansWifiChannelHelper channelLeft = YansWifiChannelHelper::Default ();
    YansWifiChannelHelper channelRight = YansWifiChannelHelper::Default ();

    //channelLeft.AddPropagationLoss ("ns3::RangePropagationLossModel");
    //channelRight.AddPropagationLoss ("ns3::RangePropagationLossModel");

    YansWifiPhyHelper phyLeft,phyRight;
    phyLeft.SetChannel (channelLeft.Create ()); 
    phyRight.SetChannel (channelRight.Create ()); 

    // MAC layer
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager"); // tells the helper the type of rate control algorithm to use, here AARF algorithm
    

    WifiMacHelper macLeft,macRight;
    Ssid ssidLeft = Ssid ("ns-3-ssid-left"); //  creates an 802.11 service set identifier (SSID) object
    Ssid ssidRight = Ssid ("ns-3-ssid-right"); 
  
    // configure Wi-Fi for all of our STA nodes
    macLeft.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssidLeft),"ActiveProbing", BooleanValue (false)); 
                
    macRight.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssidRight),"ActiveProbing", BooleanValue (false));

    NetDeviceContainer staDevicesLeft, staDevicesRight;
    staDevicesLeft = wifi.Install (phyLeft, macLeft, wifiStaNodesLeft);
    staDevicesRight = wifi.Install (phyRight, macRight, wifiStaNodesRight);

   //  configure the AP (access point) node
    macLeft.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssidLeft));
    macRight.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssidRight));

    NetDeviceContainer apDevicesLeft, apDevicesRight;
    apDevicesLeft = wifi.Install (phyLeft, macLeft, wifiApNodeLeft); // single AP which shares the same set of PHY-level Attributes (and channel) as the station
    apDevicesRight = wifi.Install (phyRight, macRight, wifiApNodeRight); 

    uint gridWidth = sqrt(nNode);
    //  Mobility Model Settings
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(0.5),
                                  "DeltaY", DoubleValue(0.5),
                                  "GridWidth", UintegerValue(gridWidth),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)),
                             "Speed", StringValue ("ns3::ConstantRandomVariable[Constant="+std::to_string(speed)+"]"));
  
    mobility.Install (wifiStaNodesLeft);
    mobility.Install (wifiStaNodesRight);

    //AP node stay still
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    // install on AP node
    mobility.Install (wifiApNodeLeft);
    mobility.Install (wifiApNodeRight);

    // Install Stack
    InternetStackHelper stack;
    stack.Install (wifiApNodeLeft);
    stack.Install (wifiApNodeRight);
    stack.Install (wifiStaNodesLeft);
    stack.Install (wifiStaNodesRight);

    // Assign IP Addresses
    Ipv4AddressHelper address;

    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign (p2pBottleNeckDevices);

    Ipv4InterfaceContainer staNodeInterfacesLeft, staNodeInterfacesRight;
    Ipv4InterfaceContainer apNodeInterfaceLeft, apNodeInterfaceRight;

    address.SetBase ("10.1.2.0", "255.255.255.0");
    staNodeInterfacesLeft = address.Assign (staDevicesLeft);
    apNodeInterfaceLeft = address.Assign (apDevicesLeft);

    address.SetBase ("10.1.3.0", "255.255.255.0");
    staNodeInterfacesRight = address.Assign (staDevicesRight);
    apNodeInterfaceRight = address.Assign (apDevicesRight);

    // Install on/off app on all left side nodes
    OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
    std::string DATA_RATE = std::to_string(PACKET_SIZE_KB * 8 * nPPS) + "kbps";
    clientHelper.SetAttribute("PacketSize", UintegerValue(1024));
    clientHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    clientHelper.SetAttribute("DataRate", StringValue(DATA_RATE));
    
    Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);

    ApplicationContainer sinkApps;
    for (int i = 0; i < nNode; ++i)
    {
        // create sink app on right side node
        sinkApps.Add (packetSinkHelper.Install (wifiStaNodesRight.Get(i)));
    }
    sinkApps.Start (Seconds (1.0));
    sinkApps.Stop (Seconds (10));


    ApplicationContainer senderApps;
    int cur_flow_count = 0;
    for (int i = 0; i < nNode; ++i)
    {
        // Create an on/off app on left side node which sends packets to the right side
        AddressValue remoteAddress (InetSocketAddress (staNodeInterfacesRight.GetAddress(i), port));
        
        for(int j = 0; j < nNode; ++j)
        {
            clientHelper.SetAttribute ("Remote", remoteAddress);
            senderApps.Add (clientHelper.Install (wifiStaNodesLeft.Get(j)));

            cur_flow_count++;
            if(cur_flow_count >= nFlow)
                break;
        }

        if(cur_flow_count >= nFlow)
            break;
    }
    senderApps.Start (Seconds (1.5)); // Start 1 second after sink
    senderApps.Stop (Seconds (9.5)); // Stop before the sink 

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (10)); // force stop,

    if(exp_name=="node")
    {
        Simulator::Schedule (Seconds (1.75), &calMetrix,2*nNode+2,exp_name);
        bottleNeckLink.EnablePcapAll("scratch/1905113/mobile/"+exp_name+"/pcap/"+exp_name+"-"+std::to_string(nNode*2+2));

    }
    else if(exp_name=="flow")
    {
        Simulator::Schedule (Seconds (1.75), &calMetrix,nFlow,exp_name);
        bottleNeckLink.EnablePcapAll("scratch/1905113/mobile/"+exp_name+"/pcap/"+exp_name+"-"+std::to_string(nFlow));

    }
    else if(exp_name=="pps")
    {
        Simulator::Schedule (Seconds (1.75), &calMetrix,nPPS,exp_name);
        bottleNeckLink.EnablePcapAll("scratch/1905113/mobile/"+exp_name+"/pcap/"+exp_name+"-"+std::to_string(nPPS));

    }
    else if(exp_name=="speed")
    {
        Simulator::Schedule (Seconds (1.75), &calMetrix,speed,exp_name);
        bottleNeckLink.EnablePcapAll("scratch/1905113/mobile/"+exp_name+"/pcap/"+exp_name+"-"+std::to_string(speed));

    }
    else{
         Simulator::Schedule (Seconds (1.75), &calMetrix,nNode,exp_name);
        bottleNeckLink.EnablePcapAll("scratch/1905113/mobile/"+exp_name+"/pcap/pcap");
    }

    for(uint32_t i=0; i<sinkApps.GetN(); i++)
    {
        Ptr<Application> app = sinkApps.Get(i);
        Ptr<PacketSink> sinkApp = DynamicCast<PacketSink>(app);

        if(sinkApp)
        {
            sinkApp->TraceConnectWithoutContext("Rx",MakeCallback(&CalculateTotalRX));
        }
    }
    for(uint32_t i=0; i<senderApps.GetN(); i++)
    {
        Ptr<Application> app = senderApps.Get(i);
        Ptr<OnOffApplication> senderApp = DynamicCast<OnOffApplication>(app);

        if (senderApp)
        {
            senderApp->TraceConnectWithoutContext("Tx", MakeCallback(&CalculateTotalTX));
        }
    }
    Simulator::Run ();

     
    double throughput = totalRx / (lastTime.GetSeconds()-initialTime.GetSeconds())/1000;
    double deliveryRatio = (totalRx * 100.00)/totalTx;

    std::cout << std::endl;
    std::cout <<"Throughput: "<< throughput << std::endl;
    std::cout <<"Delivery Ratio: "<< deliveryRatio << std::endl;
    std::cout << std::endl;

    if(exp_name=="node")
    {
        thp_out<<2*nNode+2<<" "<<throughput<<std::endl;
        dr_out<<2*nNode+2<<" "<<deliveryRatio<<std::endl;
    }
    else if(exp_name=="flow")
    {
        thp_out<<nFlow<<" "<<throughput<<std::endl;
        dr_out<<nFlow<<" "<<deliveryRatio<<std::endl;
    }
    else if(exp_name=="pps")
    {
        thp_out<<nPPS<<" "<<throughput<<std::endl;
        dr_out<<nPPS<<" "<<deliveryRatio<<std::endl;
    }
    else if(exp_name=="speed")
    {
        thp_out<<speed<<" "<<throughput<<std::endl;
        dr_out<<speed<<" "<<deliveryRatio<<std::endl;
    }

    Simulator::Destroy();
}

//NS_LOG_COMPONENT_DEFINE ("1905113_Offline2");

int main(int argc, char *argv[])
{

    std::string exp_name="default";
    std::string arrayString = "20,40,60,80,100"; // Default value

    int node=60;
    int flow=node/1.75;
    int pps=200;
    int speed=10;

    CommandLine cmd;
    cmd.AddValue("verbose", "Verbose output", verbose);
    cmd.AddValue("exp_name", "Experiment Name", exp_name);
    cmd.AddValue("integersArg", "Array of integers", arrayString);

    cmd.AddValue("node", "Number of Nodes", node);
    cmd.AddValue("flow", "Number of Flow", flow);
    cmd.AddValue("pps", "Packet Per Second", pps);
    cmd.AddValue("speed", "Speed of Node", speed);
    cmd.Parse(argc, argv);

    // Convert the comma-separated string to an array of integers
    std::vector<int> integersArg;
    std::stringstream ss(arrayString);
    std::string token;

    while (std::getline(ss, token, ',')) {
        int number = std::stoi(token);
        integersArg.push_back(number);
    }

    std::string dirSave = "mkdir -p scratch/1905113/mobile/"+exp_name;
    std::string dirDel="rm -rf scratch/1905113/mobile/"+exp_name;
    if (system (dirDel.c_str ()) == -1)
    {
        exit (1);
    }
    if (system (dirSave.c_str ()) == -1)
    {
        exit (1);
    }
    dirSave = "mkdir -p scratch/1905113/mobile/"+exp_name+"/throughput";
    if (system (dirSave.c_str ()) == -1)
    {
        exit (1);
    }
    dirSave = "mkdir -p scratch/1905113/mobile/"+exp_name+"/delivery_ratio";
    if (system (dirSave.c_str ()) == -1)
    {
        exit (1);
    }

    dirSave = "mkdir -p scratch/1905113/mobile/"+exp_name+"/pcap";
    if (system (dirSave.c_str ()) == -1)
    {
        exit (1);
    }

    thp_out.open("scratch/1905113/mobile/"+exp_name+"/throughput.dat", std::ios::out | std::ios::app);
    dr_out.open("scratch/1905113/mobile/"+exp_name+"/delivery_ratio.dat", std::ios::out | std::ios::app);

    if(exp_name=="node")
    {      
        for (size_t i=0; i < integersArg.size(); i++)
        {
            int nodeCount = integersArg[i];
            totalRx=0;
            totalTx=0;
            isInit=true;
            topology((nodeCount-2)/2,nodeCount/1.75,pps,speed,exp_name);
        }
    }
    else if(exp_name=="flow")
    {
        auto maxElement = std::max_element(integersArg.begin(), integersArg.end());
        int maxFlow;
        if (maxElement != integersArg.end())
            maxFlow = *maxElement;
        else
            maxFlow = 60;
        int nodeCount = sqrt(maxFlow);

        for (size_t i=0; i < integersArg.size(); i++)
        {
            int flowCount = integersArg[i];
            totalRx=0;
            totalTx=0;
            isInit=true;
            topology(nodeCount,flowCount,pps,speed,exp_name);
        }
    }
    else if(exp_name=="pps")
    {
        for (size_t i=0; i < integersArg.size(); i++)
        {
            pps = integersArg[i];
            totalRx=0;
            totalTx=0;
            isInit=true;
            topology((node-2)/2,flow,pps,speed,exp_name);
        }
    }
    else if(exp_name=="speed")
    {
        for (size_t i=0; i < integersArg.size(); i++)
        {
            int speed = integersArg[i];
            totalRx=0;
            totalTx=0;
            isInit=true;
            topology((node-2)/2,flow,pps,speed,exp_name);
        }
    }
    else{
        totalRx=0;
        totalTx=0;
        isInit=true;
        topology((node-2)/2,flow,pps,speed,exp_name);
    }
    thp_out.close();
    dr_out.close();
}