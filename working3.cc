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
  uint32_t nStas[2] = {25, 25};
  bool verbose = false;
  bool mixed = false;
  bool raw = false;
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

  std::vector<std::vector<uint16_t> > vecPort;
  std::vector<std::vector<ApplicationContainer> > vecApps;
  std::vector<std::vector<ApplicationContainer> > vecSinkApps;

  CommandLine cmd;
  cmd.AddValue ("nStasA", "Number of stations for wifi network A", nStas[0]);
  cmd.AddValue ("nStasB", "Number of stations for wifi network B", nStas[1]);
  cmd.AddValue ("mixed", "True if stations for both networks are mixed", mixed);
  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  
  cmd.AddValue ("packetSize", "size of application packet sent", MaxPacketSize);
  cmd.AddValue ("packetCount", "size of application packet sent", maxPacketCount);
  
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("raw", "Output raw data without labels when true", raw);
  cmd.AddValue ("rtsCts", "RTS/CTS threshold", rtsCts);

  cmd.Parse (argc, argv);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue (rtsCts));
  // Fix non-unicast data rate to be the same as that of unicast
 // Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", 
   //                   StringValue (phyMode));



// setup channel

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("RxGain", DoubleValue (0) ); 
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;// = YansWifiChannelHelper::Default();
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  InternetStackHelper stack;
  
  ApNodes.Create (nWifis);  
  stack.Install (ApNodes);  

  Ipv4AddressHelper ip;
//setup devices per network
double x= 0.0;

	if(raw)
		std::cout << nStas[0] << "\t" << nStas[1] <<"\n";

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
	  	wifi.SetStandard (WIFI_PHY_STANDARD_80211g); 

	 
	  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
	  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");


 // calculate ssid and ip range for wifi network
      std::ostringstream oss1, oss2;
      oss1 << "wifi-network-" << i;
      Ssid ssid = Ssid (oss1.str ());
      oss2 << "10."<< i << ".0.0";
      ip.SetBase (oss2.str().c_str(), "255.255.255.0");

      sta.Create (nStas[i]);


//mobility

	  // AP
	  Ptr<ListPositionAllocator> positionAllocAp = CreateObject<ListPositionAllocator> ();
	  positionAllocAp->Add (Vector (x - 1.0, 0.0, 0.0));
	  mobility.SetPositionAllocator (positionAllocAp);
     mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
     mobility.Install (ApNodes.Get (i));
    
     wifiMac.SetType ("ns3::ApWifiMac",
                       "Ssid", SsidValue (ssid));
     apDev = wifi.Install (wifiPhy, wifiMac, ApNodes.Get (i));
     apInterface = ip.Assign (apDev);


	  ObjectFactory pos;
	  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");

	  if(mixed)
	  {
			pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=-10.0|Max=10.0]"));
	  }else {
			if(x == 0)
					  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=-10.0|Max=-1.0]"));
			else
					  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=10.0]"));
		}

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

      x += 2.0;
    }  


//set up applications


	for (uint32_t i = 0; i < nWifis; ++i)
	{

	  std::vector<uint16_t> port;
	  std::vector<ApplicationContainer>apps;
	  std::vector<ApplicationContainer>sinkApps;

		for( uint16_t  a = 0; a < nStas[i]; a = a + 1 )
			{
		  		 port.push_back (8000+a);
		       Address apLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port[a]));
		       PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", apLocalAddress);
		       
				 ApplicationContainer sinkApp;
				 sinkApp = packetSinkHelper.Install (ApNodes.Get (i));
				 sinkApp.Start (Seconds (0.0));
		       sinkApp.Stop (Seconds (simulationTime+1));

				 sinkApps.push_back (sinkApp);

		       OnOffHelper onoff ("ns3::TcpSocketFactory",Ipv4Address::GetAny ());

		       onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=10]"));
		       onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
		       onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize));
		       onoff.SetAttribute ("DataRate", StringValue ("5Mbps")); //bit/s

		       AddressValue remoteAddress (InetSocketAddress (apInterfaces[i].GetAddress (0), port[a]));
		       onoff.SetAttribute ("Remote", remoteAddress);

				 ApplicationContainer app;
		       app.Add (onoff.Install (staNodes[i].Get (a)));

		       app.Start (Seconds (1.0));
		       app.Stop (Seconds (simulationTime+1));
				 apps.push_back(app); 
		}
		vecPort.push_back(port);
 		vecApps.push_back(apps);
 		vecSinkApps.push_back(sinkApps);

	}

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll(); //(staNodes[0]);

  wifiPhy.EnablePcap ("wifitcp", apDevices[0]);
  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (simulationTime+1));
  Simulator::Run ();

  monitor->CheckForLostPackets ();
  double tot=0;
  double totsq=0;
  double variance=0;
  double max = 0.0;
  double min = 9999.9;
  std::cout << std::fixed;
  
  int n = nStas[0] + nStas[1];


  double throughput[2*n]; //for every node, there are 2 flows in TCP
  int psent=0;
  int preceived=0;

  Vector pos2;


  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
	  throughput[i->first] = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024;
  //    	  std::cout << "  Throughput: " << throughput[i->first]  << " Mbps\n";
	if (t.destinationAddress=="10.0.0.1")	//only output for network A nodes
	 {
		int ncount=(i->first)-1;
		Ptr<MobilityModel> mob = staNodes[0].Get(ncount)->GetObject<MobilityModel>();
		pos2 = mob->GetPosition ();

	  std::cout << t.sourceAddress <<"\t";
	  std::cout << n << "\t" << rtsCts <<"\t";
	  std::cout << i->second.txBytes << "\t";
	  std::cout << throughput[i->first]  << "\t";
	  std::cout << pow(pow(pos2.x,2)+pow(pos2.y,2),0.5)<< "\t\n";
	  //std::cout << pos2.x  << "\t";
	  //std::cout << pos2.y  << "\t\n";


	  tot=tot+throughput[i->first];
	  if (throughput[i->first] > max)
			max = throughput[i->first];
	  if (throughput[i->first] < min)
			min = throughput[i->first];	
	  totsq=totsq+pow(throughput[i->first],2);
	  psent=psent+i->second.txBytes;
	  preceived=preceived+i->second.rxBytes;
	 }
     }


	if (raw)
	{
	  std::cout <<  tot/nStas[0] <<"\t";
	  std::cout <<  tot <<"\t";
	  variance=totsq/nStas[0]-pow(tot/nStas[0],2);

	  std::cout << pow(variance,0.5)/tot/nStas[0] <<"\t";
	  std::cout << min <<"\t";
	  std::cout << max <<"\t";
	  std::cout << psent-preceived <<"\t";
	  std::cout << psent <<"\n";


	} else {
	  std::cout << "Avg: " << tot/nStas[0] <<"\t";
	  std::cout << "Tot: " << tot <<"\t";
	  variance=totsq/nStas[0]-pow(tot/nStas[0],2);

	  std::cout << "Var: " << pow(variance,0.5)/tot/nStas[0] <<"\t";
	  std::cout << "Min: " << min <<"\t";
	  std::cout << "Max: " << max <<"\t";
	  std::cout << "loss: " << psent-preceived <<"\t";
	  std::cout << "sent: " << psent <<"\n";
	}





  Simulator::Destroy ();
  return 0;
}

