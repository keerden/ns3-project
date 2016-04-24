#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/applications-module.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>
#include <iomanip> 
#include <stdint.h>
#include <sstream>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleInfra");

int main (int argc, char *argv[])
{


  double simulationTime = 10; //seconds
  uint32_t nWifis = 2;
  uint32_t nStas = 25;
  bool verbose = false;
  uint32_t maxPacketCount = 320;
  uint32_t MaxPacketSize = 1024;
  uint32_t payloadSize = 1024;




//==========================
time_t  timev;
time(&timev);
RngSeedManager::SetSeed(timev);
RngSeedManager::SetRun (7);

//==========================

  std::string phyMode ("DsssRate11Mbps");
  std::string rtsCts ("150");

  NodeContainer ApNodes;
  std::vector<NodeContainer> staNodes;
  std::vector<NetDeviceContainer> staDevices;
  std::vector<NetDeviceContainer> apDevices;
  std::vector<Ipv4InterfaceContainer> staInterfaces;
  std::vector<Ipv4InterfaceContainer> apInterfaces;
  std::vector<Ipv4AddressHelper> ipAddresses;


  CommandLine cmd;
  cmd.AddValue ("nWifis", "Number of wifi networks", nWifis);
  cmd.AddValue ("nStas", "Number of stations per wifi network", nStas);
  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  
  cmd.AddValue ("packetSize", "size of application packet sent", MaxPacketSize);
  cmd.AddValue ("packetCount", "size of application packet sent", maxPacketCount);
  
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("rtsCts", "RTS/CTS threshold", rtsCts);

  cmd.Parse (argc, argv);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue (rtsCts));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", 
                      StringValue (phyMode));



// setup channel

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("RxGain", DoubleValue (0) ); 
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;// = YansWifiChannelHelper::Default();
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  InternetStackHelper stack;
  
  ApNodes.Create (nWifis);  
  stack.Install (ApNodes);  

  Ipv4AddressHelper ip;
//setup devices per network
double x= 0.0;



  for (uint32_t i = 0; i < nWifis; ++i)
    {
      NodeContainer sta;
      NetDeviceContainer staDev;
      NetDeviceContainer apDev;
      Ipv4InterfaceContainer staInterface;
      Ipv4InterfaceContainer apInterface;
     // Ipv4AddressHelper ip;
      MobilityHelper mobility;
     	WifiHelper wifi;

		if (verbose)
		{
			wifi.EnableLogComponents ();  // Turn on all Wifi logging
		}
	  	wifi.SetStandard (WIFI_PHY_STANDARD_80211b); 

	  // Add a non-QoS upper mac, and disable rate control
	  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
	 // wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
	//	                             "DataMode",StringValue (phyMode),
	//	                             "ControlMode",StringValue (phyMode));   //not in example!

wifi.SetRemoteStationManager ("ns3::AarfWifiManager");


 // calculate ssid and ip range for wifi network
      std::ostringstream oss1, oss2;
      oss1 << "wifi-network-" << i;
      Ssid ssid = Ssid (oss1.str ());
      oss2 << "10."<< i << ".0.0";
      ip.SetBase (oss2.str().c_str(), "255.255.255.0");

      sta.Create (nStas);


//mobility

	  // AP
	  Ptr<ListPositionAllocator> positionAllocAp = CreateObject<ListPositionAllocator> ();
	  positionAllocAp->Add (Vector (x, 0.0, 0.0));
	  mobility.SetPositionAllocator (positionAllocAp);
     mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
     mobility.Install (ApNodes.Get (i));
    
     wifiMac.SetType ("ns3::ApWifiMac",
                       "Ssid", SsidValue (ssid));
     apDev = wifi.Install (wifiPhy, wifiMac, ApNodes.Get (i));
     apInterface = ip.Assign (apDev);


	  ObjectFactory pos;
	  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");

	 
if(x == 0)
		  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=10.0]"));
else
		  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=10.0]"));

 pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=10.0]"));
	  Ptr<PositionAllocator> positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();

	  mobility.SetPositionAllocator (positionAlloc);
	  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

      // setup the STAs
      stack.Install (sta);
      mobility.Install (sta);
      wifiMac.SetType ("ns3::StaWifiMac",
                       "Ssid", SsidValue (ssid),
                       "ActiveProbing", BooleanValue (false));
      staDev = wifi.Install (wifiPhy, wifiMac, sta);
      staInterface = ip.Assign (staDev);

      // save everything in containers.
      staNodes.push_back (sta);
      apDevices.push_back (apDev);
      apInterfaces.push_back (apInterface);
      staDevices.push_back (staDev);
      staInterfaces.push_back (staInterface);

      x += 10.0;
    }  

  //port number, given in array
  uint16_t port[nStas];

  ApplicationContainer apps[nStas], sinkApp[nStas];

	for( uint16_t  a = 0; a < nStas; a = a + 1 )
   	{
	  port[a]=8000+a;
          Address apLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port[a]));
          PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", apLocalAddress);
          sinkApp[a] = packetSinkHelper.Install (ApNodes.Get (0));

          sinkApp[a].Start (Seconds (0.0));
          sinkApp[a].Stop (Seconds (simulationTime+1));
	

          OnOffHelper onoff ("ns3::TcpSocketFactory",Ipv4Address::GetAny ());

          onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=10]"));
          onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
          onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize));
          onoff.SetAttribute ("DataRate", StringValue ("5Mbps")); //bit/s

          AddressValue remoteAddress (InetSocketAddress (apInterfaces[0].GetAddress (0), port[a]));
          onoff.SetAttribute ("Remote", remoteAddress);

          apps[a].Add (onoff.Install (staNodes[0].Get (a)));

          apps[a].Start (Seconds (1.0));
          apps[a].Stop (Seconds (simulationTime+1));
	}


  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  wifiPhy.EnablePcap ("wifitcp", apDevices[0]);
  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (simulationTime+1));
  Simulator::Run ();

  monitor->CheckForLostPackets ();
  double tot=0;
  double totsq=0;
  double variance=0;
 std::cout << std::fixed;
  
  double throughput[2*nStas]; //for every node, there are 2 flows in TCP
  int psent=0;
  int preceived=0;

Vector pos2;

  ////std::cout << n << "\t" << rtsCts <<"\t";
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
          //std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\t";
          //std::cout << "  Tx Bytes:   " << i->second.txBytes << "\t";
          //std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\t";
	  throughput[i->first] = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024;
      	  //std::cout << "  Throughput: " << throughput[i->first]  << " Mbps\n";
	if (t.destinationAddress=="10.0.0.1")
	 {
int ncount=(i->first)-1;
Ptr<MobilityModel> mob = staNodes[0].Get(ncount)->GetObject<MobilityModel>();
pos2 = mob->GetPosition ();

	  std::cout << t.sourceAddress <<"\t";
	  std::cout << nStas << "\t" << rtsCts <<"\t";
	  std::cout << i->second.txBytes << "\t";
	  std::cout << throughput[i->first]  << "\t";
	  std::cout << pow(pow(pos2.x,2)+pow(pos2.y,2),0.5)<< "\t\n";
	  //std::cout << pos2.x  << "\t";
	  //std::cout << pos2.y  << "\t\n";


	  tot=tot+throughput[i->first];
	  totsq=totsq+pow(throughput[i->first],2);
	  psent=psent+i->second.txBytes;
	  preceived=preceived+i->second.rxBytes;
	 }
     }

  std::cout << "Avg: " << tot/nStas <<"\t";
  std::cout << "Tot: " << tot <<"\t";
  variance=totsq/nStas-pow(tot/nStas,2);
  //std::cout << "Variance: " << variance <<"\t";
  std::cout << "Var: " << pow(variance,0.5)/tot/nStas <<"\t";
  std::cout << "loss: " << psent-preceived <<"\t";
  std::cout << "sent: " << psent <<"\n";
  //std::cout << "Packet received: " << preceived <<"\n";
  //monitor->SerializeToXmlFile("lab-1.flowmon", true, true);

  Simulator::Destroy ();
  return 0;
}

