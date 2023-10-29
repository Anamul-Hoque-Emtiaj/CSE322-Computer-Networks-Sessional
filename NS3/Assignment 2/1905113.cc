#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/error-model.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "tutorial-app.cc"

#include <string.h>

using namespace ns3;

std::ofstream type1_ofs,type1_cwnd_ofs,fairness_index_ofs;
std::ofstream type2_ofs,type2_cwnd_ofs;
bool verbose = true;
uint32_t packetSize = 1024;
uint64_t delay = 100; // 1 ms

static void
CwndChange_type1(uint32_t oldCwnd, uint32_t newCwnd)
{
    type1_cwnd_ofs<<Simulator::Now().GetSeconds()<<" "<<newCwnd<<std::endl;
}

static void
CwndChange_type2(uint32_t oldCwnd, uint32_t newCwnd)
{
    type2_cwnd_ofs<<Simulator::Now().GetSeconds()<<" "<<newCwnd<<std::endl;
}

void topology(double bottleneckDataRate, double packetLossRate, std::string exp_name, int it)
{
    std::cout<<"Running for bottleneckDataRate: "<<bottleneckDataRate<<", packetLossRate: "<<packetLossRate<<"\n";
    if(verbose)
    {
        LogComponentEnable("PacketSink",LOG_LEVEL_INFO);
    }

    // Create point-to-point dumbbell
    PointToPointHelper p2pleaf,p2prouter;

    p2pleaf.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
    p2pleaf.SetChannelAttribute("Delay", StringValue("1ms"));

    p2prouter.SetDeviceAttribute("DataRate", StringValue(std::to_string(bottleneckDataRate)+"Mbps"));
    p2prouter.SetChannelAttribute("Delay", StringValue(std::to_string(delay)+"ms"));

    // Set router buffer capacity
    uint32_t bandwidthDelayProduct = (bottleneckDataRate*1000000 * delay*0.001) /packetSize;
    
    p2pleaf.SetQueue ("ns3::DropTailQueue", "MaxSize",
        StringValue (std::to_string (bandwidthDelayProduct) + "p"));

    // Create point-to-point dumbbell
    PointToPointDumbbellHelper dumbbell(2, p2pleaf, 2, p2pleaf, p2prouter);

    // Add rate error model to bottleneck links
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(packetLossRate));
    dumbbell.GetLeft()->GetDevice(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    dumbbell.GetRight()->GetDevice(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    InternetStackHelper stack1,stack2;
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpNewReno::GetTypeId()));
    stack1.Install(dumbbell.GetLeft());
    stack1.Install(dumbbell.GetLeft(0));
    stack1.Install(dumbbell.GetRight());
    stack1.Install(dumbbell.GetRight(0));
    
    if(it==1)
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpWestwoodPlus::GetTypeId())); 
    else if(it==2)
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpHighSpeed::GetTypeId())); 
    else
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpAdaptiveReno::GetTypeId())); 

    stack2.Install(dumbbell.GetLeft(1));
    stack2.Install(dumbbell.GetRight(1));

    Ipv4AddressHelper leftAddressHelper,rightAddressHelper,routerAddressHelper;
    leftAddressHelper.SetBase("10.1.1.0", "255.255.255.0");
    rightAddressHelper.SetBase("10.3.1.0", "255.255.255.0");
    routerAddressHelper.SetBase("10.2.1.0", "255.255.255.0");
    dumbbell.AssignIpv4Addresses(leftAddressHelper,rightAddressHelper,routerAddressHelper);


    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 8080));

    ApplicationContainer  newRenoReceiver, westWoodReceiver;
    newRenoReceiver = sinkHelper.Install(dumbbell.GetRight(0));
    newRenoReceiver.Start(Seconds(1.0));
    newRenoReceiver.Stop(Seconds(20.0));

    westWoodReceiver = sinkHelper.Install(dumbbell.GetRight(1));
    westWoodReceiver.Start(Seconds(1.0));
    westWoodReceiver.Stop(Seconds(20.0));



    // Create the newRenoSender application
    Ptr<Socket> newReno_ns3TcpSocket = Socket::CreateSocket(dumbbell.GetLeft(0), TcpSocketFactory::GetTypeId());
    newReno_ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange_type1));
    Address remoteAddress(InetSocketAddress(dumbbell.GetRightIpv4Address(0), 8080));
    Ptr<TutorialApp> newRenoSender = CreateObject<TutorialApp>();
    newRenoSender->Setup(newReno_ns3TcpSocket, remoteAddress, packetSize, 1000000, DataRate("200Mbps"));
    dumbbell.GetLeft(0)->AddApplication(newRenoSender);
    newRenoSender->SetStartTime(Seconds(1.5));
    newRenoSender->SetStopTime(Seconds(19.0));

    // Create the westWoodSender application
    Ptr<Socket> westWood_ns3TcpSocket = Socket::CreateSocket(dumbbell.GetLeft(1), TcpSocketFactory::GetTypeId());
    westWood_ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange_type2));
    Address remoteAddress2(InetSocketAddress(dumbbell.GetRightIpv4Address(1), 8080));
    Ptr<TutorialApp> westWoodSender = CreateObject<TutorialApp>();
    westWoodSender->Setup(westWood_ns3TcpSocket, remoteAddress2, packetSize, 1000000, DataRate("200Mbps"));
    dumbbell.GetLeft(1)->AddApplication(westWoodSender);
    westWoodSender->SetStartTime(Seconds(1.5));
    westWoodSender->SetStopTime(Seconds(19.0));


    // Install FlowMonitor
    FlowMonitorHelper flowMon;
    Ptr<FlowMonitor> monitor = flowMon.InstallAll();
    p2prouter.EnablePcapAll("scratch/1905113/pcap/pcap");
    // Set up the actual simulation
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    // Set the bounding box for animation
    dumbbell.BoundingBox(1, 1, 100, 100);
    // Run simulation
    Simulator::Stop(Seconds(20.0));
    Simulator::Run();

    // Calculate and print throughput
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMon.GetClassifier());

    double new_reno_thr=0,west_wood_plus_thr=0;
    for(auto itr:stats)
    {
        Ipv4FlowClassifier::FiveTuple fiveTuple = classifier->FindFlow(itr.first);
        if(fiveTuple.destinationAddress==dumbbell.GetRightIpv4Address(1) || fiveTuple.destinationAddress==dumbbell.GetLeftIpv4Address(1))
        {
            west_wood_plus_thr+=itr.second.rxBytes;
        }
        else if(fiveTuple.destinationAddress==dumbbell.GetRightIpv4Address(0) || fiveTuple.destinationAddress==dumbbell.GetLeftIpv4Address(0))
        {
            new_reno_thr+=itr.second.rxBytes;
        }
    }
    new_reno_thr = (new_reno_thr*8/Now().GetSeconds()/1000.0); //kbps
    west_wood_plus_thr = (west_wood_plus_thr*8/Now().GetSeconds()/1000.0); //kbps
    double f_ind = ((west_wood_plus_thr+new_reno_thr)*(west_wood_plus_thr+new_reno_thr))/(2.0*(west_wood_plus_thr*west_wood_plus_thr+new_reno_thr*new_reno_thr));
    if(exp_name=="bdr")
    {
        type1_ofs<<bottleneckDataRate<<" "<<new_reno_thr<<std::endl;
        type2_ofs<<bottleneckDataRate<<" "<<west_wood_plus_thr<<std::endl;
        fairness_index_ofs<<bottleneckDataRate<<" "<<f_ind<<std::endl;
    }
    else if(exp_name=="plr")
    {
        type1_ofs<<packetLossRate<<" "<<new_reno_thr<<std::endl;
        type2_ofs<<packetLossRate<<" "<<west_wood_plus_thr<<std::endl;
        fairness_index_ofs<<packetLossRate<<" "<<f_ind<<std::endl;
    }
    std::cout << " Destroying"<<"\n";
    // Cleanup
    Simulator::Destroy();

}

int main(int argc, char* argv[]) {
    CommandLine cmd;
    std::string exp_name="cwnd";
    //std::string exp_name="bdr";
    std::string arrayString = "1,50,100,150,200,250,300";
    
    cmd.AddValue("verbose", "Verbose output", verbose);
    cmd.AddValue("exp_name", "Experiment Name", exp_name);
    cmd.AddValue("numbersArg", "Array of Numbers", arrayString);
    cmd.Parse(argc, argv);

    std::vector<double> numbersArg;
    std::stringstream ss(arrayString);
    std::string token;

    while (std::getline(ss, token, ',')) {
        double number = std::stod(token);
        numbersArg.push_back(number);
    }
    
    for(int i=1; i<=3; i++)
    {
        std::string exp;
        if(i==1)
            exp = "newreno-vs-westwoodplus";
        else if(i==2)
            exp = "newreno-vs-highspeed";
        else
            exp = "newreno-vs-adaptivereno";

        type1_ofs.open("scratch/1905113/"+exp_name+"/"+exp+"/1st_type.dat", std::ios::out | std::ios::app);
        type2_ofs.open("scratch/1905113/"+exp_name+"/"+exp+"/2nd_type.dat", std::ios::out | std::ios::app);
        type1_cwnd_ofs.open("scratch/1905113/"+exp_name+"/"+exp+"/1st_type_cwnd.dat", std::ios::out | std::ios::app);
        type2_cwnd_ofs.open("scratch/1905113/"+exp_name+"/"+exp+"/2nd_type_cwnd.dat", std::ios::out | std::ios::app);
        fairness_index_ofs.open("scratch/1905113/"+exp_name+"/"+exp+"/fairness_index.dat", std::ios::out | std::ios::app);

        if(exp_name=="bdr")
        {  
                
            for (size_t i=0; i < numbersArg.size(); i++)
            {
                double bdr = numbersArg[i];
                topology(bdr,0.000001,exp_name,i);
            }
        }
        else if(exp_name=="plr")
        {
            for (size_t i=0; i < numbersArg.size(); i++)
            {
                double plr = numbersArg[i];
                topology(50,plr,exp_name,i);
            }
        }
        else
        {
            topology(5000,0.00001,exp_name,i);
        }
        type1_ofs.close();
        type2_ofs.close();
        type1_cwnd_ofs.close();
        type2_cwnd_ofs.close();
        fairness_index_ofs.close();
    }
    return 0;
}