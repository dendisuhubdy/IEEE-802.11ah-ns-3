/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 MIRKO BANCHI
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "s1g-mac-test.h"

NS_LOG_COMPONENT_DEFINE ("s1g-wifi-network-le");

uint32_t AssocNum = 0;
int64_t AssocTime=0;
uint32_t StaNum=0;
//string traffic_filepath;
//uint32_t payloadLength;
NetDeviceContainer staDeviceCont;
const int MaxSta = 8000;

Configuration config;
Statistics stats;
SimulationEventManager eventManager;

class assoc_record
{
public:
    assoc_record ();
    bool GetAssoc ();
    void SetAssoc (std::string context, Mac48Address address);
    void UnsetAssoc (std::string context, Mac48Address address);
    void setstaid (uint16_t id);
 private:
    bool assoc;
    uint16_t staid;
};

assoc_record::assoc_record ()
{
    assoc = false;
    staid = 65535;
}

void
assoc_record::setstaid (uint16_t id)
{
    staid = id;
}

void
assoc_record::SetAssoc (std::string context, Mac48Address address)
{
   assoc = true;
}

void
assoc_record::UnsetAssoc (std::string context, Mac48Address address)
{
  assoc = false;
}

bool
assoc_record::GetAssoc ()
{
    return assoc;
}

typedef std::vector <assoc_record * > assoc_recordVector;
assoc_recordVector assoc_vector;


uint32_t
GetAssocNum ( )
{
  AssocNum = 0;
  for (assoc_recordVector::const_iterator index = assoc_vector.begin (); index != assoc_vector.end (); index++)
    {
      if ((*index)->GetAssoc ())
        {
            AssocNum++;
        }
    }
  return AssocNum;
}

/*
void
UdpTraffic ( uint16_t num, uint32_t Size, string path, NetDeviceContainer staDevice)
{
  StaNum = num;
  payloadLength = Size;
  traffic_filepath = path;
  staDeviceCont = staDevice;
}*/

ApplicationContainer serverApp;
uint32_t AppStartTime = 0;
uint32_t ApStopTime = 0;

std::map<uint16_t, float> traffic_sta;



/*
void CheckAssoc (uint32_t Nsta, double simulationTime, NodeContainer wifiApNode, NodeContainer  wifiStaNode, Ipv4InterfaceContainer apNodeInterface)
{
    if  (GetAssocNum () == Nsta)
      {
        std::cout << "Assoc Finished, AssocNum=" << AssocNum << ", time = " << Simulator::Now ().GetMicroSeconds () << std::endl;
        //Application start time
        Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable> ();

        UdpClientHelper myClient (apNodeInterface.GetAddress (0), 9); //address of remote node
        myClient.SetAttribute ("MaxPackets", config.maxNumberOfPackets);
        myClient.SetAttribute ("PacketSize", UintegerValue (config.payloadSize));

          traffic_sta.clear ();
          
          ifstream trafficfile (traffic_filepath);
          
           if (trafficfile.is_open())
                 {
                  uint16_t sta_id;
                  float  sta_traffic;
                  for (uint16_t kk=0; kk< Nsta; kk++)
                    {
                      trafficfile >> sta_id;
                      trafficfile >> sta_traffic;
                      traffic_sta.insert (std::make_pair(sta_id,sta_traffic)); //insert data
                      //cout << "sta_id = " << sta_id << " sta_traffic = " << sta_traffic << "\n";
                    }
                  trafficfile.close();
                }
           else cout << "Unable to open file \n";
          
          double randomStart = 0.0;
          
          for (std::map<uint16_t,float>::iterator it=traffic_sta.begin(); it!=traffic_sta.end(); ++it)
              {
                  std::ostringstream intervalstr;
                  intervalstr << (payloadLength*8)/(it->second * 1000000);
                  std::string intervalsta = intervalstr.str();

                  //config.trafficInterval = UintegerValue (Time (intervalsta));

                  myClient.SetAttribute ("Interval", TimeValue (Time (intervalsta))); //packets/s
                  randomStart = m_rv->GetValue (0, (payloadLength*8)/(it->second * 1000000));
                  ApplicationContainer clientApp = myClient.Install (wifiStaNode.Get(it->first));

                  clientApp.Start (Seconds (1 + randomStart));
                  clientApp.Stop (Seconds (simulationTime+1)); //
               }
              AppStartTime=Simulator::Now ().GetSeconds () + 1;
              Simulator::Stop (Seconds (simulationTime+1));
      }
    else
      {
        Simulator::Schedule(Seconds(1.0), &CheckAssoc, Nsta, simulationTime, wifiApNode, wifiStaNode, apNodeInterface);
      }
}*/



void
PopulateArpCache ()
{
    Ptr<ArpCache> arp = CreateObject<ArpCache> ();
    arp->SetAliveTimeout (Seconds(3600 * 24 * 365));
    for (NodeList::Iterator i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Ipv4L3Protocol> ip = (*i)->GetObject<Ipv4L3Protocol> ();
        NS_ASSERT(ip !=0);
        ObjectVectorValue interfaces;
        ip->GetAttribute("InterfaceList", interfaces);
        for(ObjectVectorValue::Iterator j = interfaces.Begin(); j !=
            interfaces.End (); j ++)
        {
            Ptr<Ipv4Interface> ipIface = (j->second)->GetObject<Ipv4Interface> ();
            NS_ASSERT(ipIface != 0);
            Ptr<NetDevice> device = ipIface->GetDevice();
            NS_ASSERT(device != 0);
            Mac48Address addr = Mac48Address::ConvertFrom(device->GetAddress ());
            for(uint32_t k = 0; k < ipIface->GetNAddresses (); k ++)
            {
                Ipv4Address ipAddr = ipIface->GetAddress (k).GetLocal();
                if(ipAddr == Ipv4Address::GetLoopback())
                    continue;
                ArpCache::Entry * entry = arp->Add(ipAddr);
                entry->MarkWaitReply(0);
                entry->MarkAlive(addr);
                std::cout << "Arp Cache: Adding the pair (" << addr << "," << ipAddr << ")" << std::endl;
            }
        }
    }
    for (NodeList::Iterator i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Ipv4L3Protocol> ip = (*i)->GetObject<Ipv4L3Protocol> ();
        NS_ASSERT(ip !=0);
        ObjectVectorValue interfaces;
        ip->GetAttribute("InterfaceList", interfaces);
        for(ObjectVectorValue::Iterator j = interfaces.Begin(); j !=interfaces.End (); j ++)
        {
            Ptr<Ipv4Interface> ipIface = (j->second)->GetObject<Ipv4Interface> ();
            ipIface->SetAttribute("ArpCache", PointerValue(arp));
        }
    }
}

uint16_t ngroup;
uint16_t nslot;
RPSVector configureRAW (RPSVector rpslist, string RAWConfigFile)
{
    uint16_t NRPS = 0;
    uint16_t NRAWPERBEACON = 0;
    uint16_t Value = 0;
    uint32_t page = 0;
    uint32_t aid_start = 0;
    uint32_t aid_end = 0;
    uint32_t rawinfo = 0;

    ifstream myfile (RAWConfigFile);
    //1. get info from config file

    //2. define RPS
    if (myfile.is_open())
    {
        myfile >> NRPS;

        for (uint16_t kk=0; kk< NRPS; kk++)   // number of beacons covering all raw groups
        {
            RPS *m_rps = new RPS;
            myfile >> NRAWPERBEACON;
            ngroup = NRAWPERBEACON;
            for (uint16_t i=0; i< NRAWPERBEACON; i++)  // raw groups in one beacon
            {
                //RPS *m_rps = new RPS;
                RPS::RawAssignment *m_raw = new RPS::RawAssignment;

                myfile >> Value;
                m_raw->SetRawControl (Value);//support paged STA or not
                myfile >> Value;
                m_raw->SetSlotCrossBoundary (Value);
                myfile >> Value;
                m_raw->SetSlotFormat (Value);
                myfile >> Value;
                m_raw->SetSlotDurationCount (Value);
                myfile >> Value;
                nslot = Value;
                m_raw->SetSlotNum (Value);
                myfile >> page;
                myfile >> aid_start;
                myfile >> aid_end;
                rawinfo = (aid_end << 13) | (aid_start << 2) | page;
                m_raw->SetRawGroup (rawinfo);

                m_rps->SetRawAssignment(*m_raw);
                delete m_raw;
            }
            rpslist.rpsset.push_back (m_rps);
            //config.nRawGroupsPerRpsList.push_back(NRAWPERBEACON);
        }
        myfile.close();
        config.NRawSta = rpslist.rpsset[rpslist.rpsset.size()-1]->GetRawAssigmentObj(NRAWPERBEACON-1).GetRawGroupAIDEnd();
    }
    else cout << "Unable to open file \n";

    return rpslist;
}


void sendStatistics(bool schedule) {
	eventManager.onUpdateStatistics(stats);
	eventManager.onUpdateSlotStatistics(transmissionsPerTIMGroupAndSlotFromAPSinceLastInterval, transmissionsPerTIMGroupAndSlotFromSTASinceLastInterval);
	// reset
	std::fill(transmissionsPerTIMGroupAndSlotFromAPSinceLastInterval.begin(), transmissionsPerTIMGroupAndSlotFromAPSinceLastInterval.end(), 0);
	std::fill(transmissionsPerTIMGroupAndSlotFromSTASinceLastInterval.begin(), transmissionsPerTIMGroupAndSlotFromSTASinceLastInterval.end(), 0);

	if(schedule)
		Simulator::Schedule(Seconds(config.visualizerSamplingInterval), &sendStatistics, true);
}

void onSTADeassociated(int i) {
	eventManager.onNodeDeassociated(*nodes[i]);
}

void updateNodesQueueLength() {
	for (uint32_t i = 0; i < config.Nsta; i++) {
		nodes[i]->UpdateQueueLength();
		stats.get(i).EDCAQueueLength = nodes[i]->queueLength;
	}
	Simulator::Schedule(Seconds(0.5), &updateNodesQueueLength);
}

void onSTAAssociated(int i) {
	cout << "Node " << std::to_string(i) << " is associated and has aid " << nodes[i]->aId << endl;
	/*
	uint32_t nrOfSTAAssociated (0);
	for (uint32_t i = 0; i < config.Nsta; i++) {
		if (nodes[i]->isAssociated)
			nrOfSTAAssociated++;
	}*/

	for (int k = 0; k < config.rps.rpsset.size(); k++)
	{
		for (int j=0; j < config.rps.rpsset[k]->GetNumberOfRawGroups(); j++)
		{
			if (config.rps.rpsset[k]->GetRawAssigmentObj(j).GetRawGroupAIDStart() <= i+1 && i+1 <= config.rps.rpsset[k]->GetRawAssigmentObj(j).GetRawGroupAIDEnd())
			{
				nodes[i]->rpsIndex = k + 1;
				nodes[i]->rawGroupNumber = j + 1;
				nodes[i]->rawSlotIndex = nodes[i]->aId % config.rps.rpsset[k]->GetRawAssigmentObj(j).GetSlotNum() + 1;
				/*cout << "Node " << i << " with AID " << (int)nodes[i]->aId << " belongs to " << (int)nodes[i]->rawSlotIndex << " slot of RAW group "
	                                               << (int)nodes[i]->rawGroupNumber << " within the " << (int)nodes[i]->rpsIndex << " RPS." << endl;
				 */
			}
		}
	}

	eventManager.onNodeAssociated(*nodes[i]);

	// RPS, Raw group and RAW slot assignment
	//stats.numAssoc = GetAssocNum ( );

	if (GetAssocNum ( ) == config.Nsta) {
		cout << "All stations associated, configuring clients & server" << endl;
        // association complete, start sending packets
    	stats.TimeWhenEverySTAIsAssociated = Simulator::Now();

    	if(config.trafficType == "udp") {
    		configureUDPServer();
    		configureUDPClients();
    	}
    	else if(config.trafficType == "udpecho") {
    		configureUDPEchoServer();
    		configureUDPEchoClients();
    	}
    	else if(config.trafficType == "tcpecho") {
			configureTCPEchoServer();
			configureTCPEchoClients();
    	}
    	else if(config.trafficType == "tcppingpong") {
    		configureTCPPingPongServer();
			configureTCPPingPongClients();
    	}
    	else if(config.trafficType == "tcpipcamera") {
    		config.ipcameraMotionPercentage = 1;// = 1; //0.1
    		config.ipcameraMotionDuration = 10;// = 10; //60
    		config.ipcameraDataRate = 128;// = 128; //20
    		config.MinRTO = 81920000;// 81920000; //819200
    		config.TCPConnectionTimeout = 6000000;
    		config.TCPSegmentSize = 3216;//  = 3216; //536
    		config.TCPInitialSlowStartThreshold = 0xffff;
    		config.TCPInitialCwnd = 1;

    		configureTCPIPCameraServer();
    		configureTCPIPCameraClients();
    	}
    	else if(config.trafficType == "tcpfirmware") {
    		config.firmwareSize = 1024 * 500;
    		config.firmwareBlockSize = 1024;
    		config.firmwareNewUpdateProbability = 0.01;
    		config.firmwareCorruptionProbability = 0.01;
    		config.firmwareVersionCheckInterval = 1000;

			configureTCPIPCameraServer();
			configureTCPIPCameraClients();
		}
    	else if(config.trafficType == "tcpfirmware") {
            
            config.firmwareSize = 1024 * 500;
     		config.firmwareBlockSize = 1024;
     		config.firmwareNewUpdateProbability = 0.01;
     		config.firmwareCorruptionProbability = 0.01;
     		config.firmwareVersionCheckInterval = 1000;
 
			configureTCPFirmwareServer();
			configureTCPFirmwareClients();
		}
    	else if(config.trafficType == "tcpsensor") {
			configureTCPSensorServer();
			configureTCPSensorClients();
    	}
    	/*else if (config.trafficType == "coap") {
			configureCoapServer();
			configureCoapClients();
		}*/
        updateNodesQueueLength();
    }
}

void RpsIndexTrace (uint16_t oldValue, uint16_t newValue)
{
	currentRps = newValue;
	//cout << "RPS: " << newValue << " at " << Simulator::Now().GetMicroSeconds() << endl;
}

void RawGroupTrace (uint8_t oldValue, uint8_t newValue)
{
	currentRawGroup = newValue;
	//cout << "	group " << std::to_string(newValue) << " at " << Simulator::Now().GetMicroSeconds() << endl;
}

void RawSlotTrace (uint8_t oldValue, uint8_t newValue)
{
	currentRawSlot = newValue;
	//cout << "		slot " << std::to_string(newValue) << " at " << Simulator::Now().GetMicroSeconds() << endl;
}

void configureNodes(NodeContainer& wifiStaNode, NetDeviceContainer& staDevice) {
	cout << "Configuring STA Node trace sources" << endl;

    for (uint32_t i = 0; i < config.Nsta; i++) {

    	cout << "Hooking up trace sources for STA " << i << endl;

        NodeEntry* n = new NodeEntry(i, &stats, wifiStaNode.Get(i), staDevice.Get(i));

        n->SetAssociatedCallback([ = ]{onSTAAssociated(i);});
        n->SetDeassociatedCallback([ = ]{onSTADeassociated(i);});
        
        nodes.push_back(n);
        // hook up Associated and Deassociated events
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/Assoc", MakeCallback(&NodeEntry::SetAssociation, n));
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/DeAssoc", MakeCallback(&NodeEntry::UnsetAssociation, n));
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/NrOfTransmissionsDuringRAWSlot", MakeCallback(&NodeEntry::OnNrOfTransmissionsDuringRAWSlotChanged, n));//not implem

        //Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/S1gBeaconMissed", MakeCallback(&NodeEntry::OnS1gBeaconMissed, n));

        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/PacketDropped", MakeCallback(&NodeEntry::OnMacPacketDropped, n));
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/Collision", MakeCallback(&NodeEntry::OnCollision, n));
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/TransmissionWillCrossRAWBoundary", MakeCallback(&NodeEntry::OnTransmissionWillCrossRAWBoundary, n)); //?

        // hook up TX
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyTxBegin", MakeCallback(&NodeEntry::OnPhyTxBegin, n));
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyTxEnd", MakeCallback(&NodeEntry::OnPhyTxEnd, n));
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyTxDropWithReason", MakeCallback(&NodeEntry::OnPhyTxDrop, n));//?

        // hook up RX
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyRxBegin", MakeCallback(&NodeEntry::OnPhyRxBegin, n));
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyRxEnd", MakeCallback(&NodeEntry::OnPhyRxEnd, n));
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyRxDropWithReason", MakeCallback(&NodeEntry::OnPhyRxDrop, n));

        // hook up MAC traces
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/RemoteStationManager/MacTxRtsFailed", MakeCallback(&NodeEntry::OnMacTxRtsFailed, n)); //?
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/RemoteStationManager/MacTxDataFailed", MakeCallback(&NodeEntry::OnMacTxDataFailed, n));
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/RemoteStationManager/MacTxFinalRtsFailed", MakeCallback(&NodeEntry::OnMacTxFinalRtsFailed, n)); //?
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/RemoteStationManager/MacTxFinalDataFailed", MakeCallback(&NodeEntry::OnMacTxFinalDataFailed, n));//?

        // hook up PHY State change
        Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Phy/State/State", MakeCallback(&NodeEntry::OnPhyStateChange, n));

    }
}

int getBandwidth(string dataMode) {
	if(dataMode == "MCS1_0" ||
	   dataMode == "MCS1_1" || dataMode == "MCS1_2" ||
		dataMode == "MCS1_3" || dataMode == "MCS1_4" ||
		dataMode == "MCS1_5" || dataMode == "MCS1_6" ||
		dataMode == "MCS1_7" || dataMode == "MCS1_8" ||
		dataMode == "MCS1_9" || dataMode == "MCS1_10")
			return 1;

	else if(dataMode == "MCS2_0" ||
	    dataMode == "MCS2_1" || dataMode == "MCS2_2" ||
		dataMode == "MCS2_3" || dataMode == "MCS2_4" ||
		dataMode == "MCS2_5" || dataMode == "MCS2_6" ||
		dataMode == "MCS2_7" || dataMode == "MCS2_8")
			return 2;

	return 0;
}

string getWifiMode(string dataMode) {
	if(dataMode == "MCS1_0") return "OfdmRate300KbpsBW1MHz";
	else if(dataMode == "MCS1_1") return "OfdmRate600KbpsBW1MHz";
	else if(dataMode == "MCS1_2") return "OfdmRate900KbpsBW1MHz";
	else if(dataMode == "MCS1_3") return "OfdmRate1_2MbpsBW1MHz";
	else if(dataMode == "MCS1_4") return "OfdmRate1_8MbpsBW1MHz";
	else if(dataMode == "MCS1_5") return "OfdmRate2_4MbpsBW1MHz";
	else if(dataMode == "MCS1_6") return "OfdmRate2_7MbpsBW1MHz";
	else if(dataMode == "MCS1_7") return "OfdmRate3MbpsBW1MHz";
	else if(dataMode == "MCS1_8") return "OfdmRate3_6MbpsBW1MHz";
	else if(dataMode == "MCS1_9") return "OfdmRate4MbpsBW1MHz";
	else if(dataMode == "MCS1_10") return "OfdmRate150KbpsBW1MHz";


	else if(dataMode == "MCS2_0") return "OfdmRate650KbpsBW2MHz";
	else if(dataMode == "MCS2_1") return "OfdmRate1_3MbpsBW2MHz";
	else if(dataMode == "MCS2_2") return "OfdmRate1_95MbpsBW2MHz";
	else if(dataMode == "MCS2_3") return "OfdmRate2_6MbpsBW2MHz";
	else if(dataMode == "MCS2_4") return "OfdmRate3_9MbpsBW2MHz";
	else if(dataMode == "MCS2_5") return "OfdmRate5_2MbpsBW2MHz";
	else if(dataMode == "MCS2_6") return "OfdmRate5_85MbpsBW2MHz";
	else if(dataMode == "MCS2_7") return "OfdmRate6_5MbpsBW2MHz";
	else if(dataMode == "MCS2_8") return "OfdmRate7_8MbpsBW2MHz";
	return "";
}

void OnAPPhyRxDrop(std::string context, Ptr<const Packet> packet, DropReason reason) {
	// THIS REQUIRES PACKET METADATA ENABLE!
	auto pCopy = packet->Copy();
	auto it = pCopy->BeginItem();
	while(it.HasNext()) {

		auto item = it.Next();
		Callback<ObjectBase *> constructor = item.tid.GetConstructor ();

		ObjectBase *instance = constructor ();
		Chunk *chunk = dynamic_cast<Chunk *> (instance);
		chunk->Deserialize (item.current);

		if(dynamic_cast<WifiMacHeader*>(chunk)) {
			WifiMacHeader* hdr = (WifiMacHeader*)chunk;

			int staId = -1;
			if (!config.useV6){
				for (uint32_t i = 0; i < staNodeInterface.GetN(); i++) {
					if(wifiStaNode.Get(i)->GetDevice(0)->GetAddress() == hdr->GetAddr2()) {
						staId = i;
						break;
					}
				}
			}
			else
			{
				for (uint32_t i = 0; i < staNodeInterface6.GetN(); i++) {
					if(wifiStaNode.Get(i)->GetDevice(0)->GetAddress() == hdr->GetAddr2()) {
						staId = i;
						break;
					}
				}
			}
			if(staId != -1) {
				stats.get(staId).NumberOfDropsByReasonAtAP[reason]++;
			}
			delete chunk;
			break;
		}
		else
			delete chunk;
	}


}

void OnAPPacketToTransmitReceived(string context, Ptr<const Packet> packet, Mac48Address to, bool isScheduled, bool isDuringSlotOfSTA, Time timeLeftInSlot) {
	int staId = -1;
	if (!config.useV6){
		for (uint32_t i = 0; i < staNodeInterface.GetN(); i++) {
			if(wifiStaNode.Get(i)->GetDevice(0)->GetAddress() == to) {
				staId = i;
				break;
			}
		}
	}
	else
	{
		for (uint32_t i = 0; i < staNodeInterface6.GetN(); i++) {
			if(wifiStaNode.Get(i)->GetDevice(0)->GetAddress() == to) {
				staId = i;
				break;
			}
		}
	}
	if(staId != -1) {
		if(isScheduled)
			stats.get(staId).NumberOfAPScheduledPacketForNodeInNextSlot++;
		else {
			stats.get(staId).NumberOfAPSentPacketForNodeImmediately++;
			stats.get(staId).APTotalTimeRemainingWhenSendingPacketInSameSlot += timeLeftInSlot;
		}
	}
}

void onChannelTransmission(Ptr<NetDevice> senderDevice, Ptr<Packet> packet) {
	int rpsIndex = currentRps - 1;
	int rawGroup = currentRawGroup - 1;
	int slotIndex = currentRawSlot - 1;
	//cout << rpsIndex << "		" << rawGroup << "		" << slotIndex << "		" << endl;

	uint64_t iSlot = slotIndex;
	if (rpsIndex > 0)
		for (int r = rpsIndex - 1; r >= 0; r--)
			for (int g = 0; g < config.rps.rpsset[r]->GetNumberOfRawGroups(); g++)
				iSlot += config.rps.rpsset[r]->GetRawAssigmentObj(g).GetSlotNum();

	if (rawGroup > 0)
		for (int i = rawGroup - 1; i >= 0; i--)
			iSlot += config.rps.rpsset[rpsIndex]->GetRawAssigmentObj(i).GetSlotNum();

	if (rpsIndex >= 0 && rawGroup >= 0 && slotIndex >= 0)
		if(senderDevice->GetAddress() == apDevice.Get(0)->GetAddress()) {
			// from AP
			transmissionsPerTIMGroupAndSlotFromAPSinceLastInterval[iSlot]+= packet->GetSerializedSize();
		}
		else {
			// from STA
			transmissionsPerTIMGroupAndSlotFromSTASinceLastInterval[iSlot]+= packet->GetSerializedSize();

		}
}

int getSTAIdFromAddress(Ipv4Address from) {
    int staId = -1;
    for (int i = 0; i < staNodeInterface.GetN(); i++) {
        if (staNodeInterface.GetAddress(i) == from) {
            staId = i;
            break;
        }
    }
    return staId;
}

void udpPacketReceivedAtServer(Ptr<const Packet> packet, Address from) { //works
	//cout << "+++++++++++udpPacketReceivedAtServer" << endl;
    int staId = getSTAIdFromAddress(InetSocketAddress::ConvertFrom(from).GetIpv4());
    if (staId != -1)
        nodes[staId]->OnUdpPacketReceivedAtAP(packet);
    else
    	cout << "*** Node could not be determined from received packet at AP " << endl;
}

void tcpPacketReceivedAtServer (Ptr<const Packet> packet, Address from) {
	int staId = getSTAIdFromAddress(InetSocketAddress::ConvertFrom(from).GetIpv4());
    if (staId != -1)
        nodes[staId]->OnTcpPacketReceivedAtAP(packet);
    else
    	cout << "*** Node could not be determined from received packet at AP " << endl;
}

void tcpRetransmissionAtServer(Address to) {
	int staId = getSTAIdFromAddress(Ipv4Address::ConvertFrom(to));
	if (staId != -1)
		nodes[staId]->OnTcpRetransmissionAtAP();
	else
		cout << "*** Node could not be determined from received packet at AP " << endl;
}

void tcpPacketDroppedAtServer(Address to, Ptr<Packet> packet, DropReason reason) {
	int staId = getSTAIdFromAddress(Ipv4Address::ConvertFrom(to));
	if(staId != -1) {
		stats.get(staId).NumberOfDropsByReasonAtAP[reason]++;
	}
}

void tcpStateChangeAtServer(TcpSocket::TcpStates_t oldState, TcpSocket::TcpStates_t newState, Address to) {

    int staId = getSTAIdFromAddress(InetSocketAddress::ConvertFrom(to).GetIpv4());
    if(staId != -1)
			nodes[staId]->OnTcpStateChangedAtAP(oldState, newState);
		else
			cout << "*** Node could not be determined from received packet at AP " << endl;

	//cout << Simulator::Now().GetMicroSeconds() << " ********** TCP SERVER SOCKET STATE CHANGED FROM " << oldState << " TO " << newState << endl;
}

void tcpIPCameraDataReceivedAtServer(Address from, uint16_t nrOfBytes) {
    int staId = getSTAIdFromAddress(InetSocketAddress::ConvertFrom(from).GetIpv4());
    if(staId != -1)
			nodes[staId]->OnTcpIPCameraDataReceivedAtAP(nrOfBytes);
		else
			cout << "*** Node could not be determined from received packet at AP " << endl;
}

void configureUDPServer() {
    UdpServerHelper myServer(9);
    serverApp = myServer.Install(wifiApNode);
    serverApp.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&udpPacketReceivedAtServer));
    serverApp.Start(Seconds(0));

}

void configureUDPEchoServer() {
    UdpEchoServerHelper myServer(9);
    serverApp = myServer.Install(wifiApNode);
    //serverApp.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&udpPacketReceivedAtServer));
    serverApp.Start(Seconds(0));
}

void configureTCPEchoServer() {
	TcpEchoServerHelper myServer(80);
	serverApp = myServer.Install(wifiApNode);
	wireTCPServer(serverApp);
	serverApp.Start(Seconds(0));
}


void configureTCPPingPongServer() {
	// TCP ping pong is a test for the new base tcp-client and tcp-server applications
	ObjectFactory factory;
	factory.SetTypeId (TCPPingPongServer::GetTypeId ());
	factory.Set("Port", UintegerValue (81));

	Ptr<Application> tcpServer = factory.Create<TCPPingPongServer>();
	wifiApNode.Get(0)->AddApplication(tcpServer);

	auto serverApp = ApplicationContainer(tcpServer);
	wireTCPServer(serverApp);
	serverApp.Start(Seconds(0));
}

void configureTCPPingPongClients() {

	ObjectFactory factory;
	factory.SetTypeId (TCPPingPongClient::GetTypeId ());
	factory.Set("Interval", TimeValue(MilliSeconds(config.trafficInterval)));
	factory.Set("PacketSize", UintegerValue(config.payloadSize));

	factory.Set("RemoteAddress", Ipv4AddressValue (apNodeInterface.GetAddress(0)));
	factory.Set("RemotePort", UintegerValue (81));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable> ();

	for (uint16_t i = 0; i < config.Nsta; i++) {

		Ptr<Application> tcpClient = factory.Create<TCPPingPongClient>();
		wifiStaNode.Get(i)->AddApplication(tcpClient);
		auto clientApp = ApplicationContainer(tcpClient);
		wireTCPClient(clientApp,i);

		double random = m_rv->GetValue(0, config.trafficInterval);
		clientApp.Start(MilliSeconds(0+random));
		//clientApp.Stop(Seconds(simulationTime + 1));
	}
}


void configureTCPIPCameraServer() {
	ObjectFactory factory;
	factory.SetTypeId (TCPIPCameraServer::GetTypeId ());
	factory.Set("Port", UintegerValue (82));

	Ptr<Application> tcpServer = factory.Create<TCPIPCameraServer>();
	wifiApNode.Get(0)->AddApplication(tcpServer);


	auto serverApp = ApplicationContainer(tcpServer);
	wireTCPServer(serverApp);
	serverApp.Start(Seconds(0));
//	serverApp.Stop(Seconds(config.simulationTime));
}

void configureTCPIPCameraClients() {

	ObjectFactory factory;
	factory.SetTypeId (TCPIPCameraClient::GetTypeId ());
	factory.Set("MotionPercentage", DoubleValue(config.ipcameraMotionPercentage));
	factory.Set("MotionDuration", TimeValue(Seconds(config.ipcameraMotionDuration)));
	factory.Set("DataRate", UintegerValue(config.ipcameraDataRate));

	factory.Set("PacketSize", UintegerValue(config.payloadSize));

	factory.Set("RemoteAddress", Ipv4AddressValue (apNodeInterface.GetAddress(0)));
	factory.Set("RemotePort", UintegerValue (82));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable> ();

	for (uint16_t i = 0; i < config.Nsta; i++) {

		Ptr<Application> tcpClient = factory.Create<TCPIPCameraClient>();
		wifiStaNode.Get(i)->AddApplication(tcpClient);
		auto clientApp = ApplicationContainer(tcpClient);
		wireTCPClient(clientApp,i);

		clientApp.Start(MilliSeconds(0));
		//clientApp.Stop(Seconds(config.simulationTime));
	}
}



void configureTCPFirmwareServer() {
	ObjectFactory factory;
	factory.SetTypeId (TCPFirmwareServer::GetTypeId ());
	factory.Set("Port", UintegerValue (83));

	factory.Set("FirmwareSize", UintegerValue (config.firmwareSize));
	factory.Set("BlockSize", UintegerValue (config.firmwareBlockSize));
	factory.Set("NewUpdateProbability", DoubleValue (config.firmwareNewUpdateProbability));

	Ptr<Application> tcpServer = factory.Create<TCPFirmwareServer>();
	wifiApNode.Get(0)->AddApplication(tcpServer);


	auto serverApp = ApplicationContainer(tcpServer);
	wireTCPServer(serverApp);
	serverApp.Start(Seconds(0));
//	serverApp.Stop(Seconds(config.simulationTime));
}

void configureTCPFirmwareClients() {

	ObjectFactory factory;
	factory.SetTypeId (TCPFirmwareClient::GetTypeId ());
	factory.Set("CorruptionProbability", DoubleValue(config.firmwareCorruptionProbability));
	factory.Set("VersionCheckInterval", TimeValue(MilliSeconds(config.firmwareVersionCheckInterval)));
	factory.Set("PacketSize", UintegerValue(config.payloadSize));

	factory.Set("RemoteAddress", Ipv4AddressValue (apNodeInterface.GetAddress(0)));
	factory.Set("RemotePort", UintegerValue (83));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable> ();

	for (uint16_t i = 0; i < config.Nsta; i++) {

		Ptr<Application> tcpClient = factory.Create<TCPFirmwareClient>();
		wifiStaNode.Get(i)->AddApplication(tcpClient);
		auto clientApp = ApplicationContainer(tcpClient);
		wireTCPClient(clientApp,i);

		double random = m_rv->GetValue(0, config.trafficInterval);
		clientApp.Start(MilliSeconds(0+random));
		clientApp.Stop(Seconds(config.simulationTime));
	}
}


void configureTCPSensorServer() {
	ObjectFactory factory;
	factory.SetTypeId (TCPSensorServer::GetTypeId ());
	factory.Set("Port", UintegerValue (84));

	Ptr<Application> tcpServer = factory.Create<TCPSensorServer>();
	wifiApNode.Get(0)->AddApplication(tcpServer);


	auto serverApp = ApplicationContainer(tcpServer);
	wireTCPServer(serverApp);
	serverApp.Start(Seconds(0));
//	serverApp.Stop(Seconds(config.simulationTime));
}

void configureTCPSensorClients() {

	ObjectFactory factory;
	factory.SetTypeId (TCPSensorClient::GetTypeId ());

	factory.Set("Interval", TimeValue(MilliSeconds(config.trafficInterval)));
	factory.Set("PacketSize", UintegerValue(config.payloadSize));
	factory.Set("MeasurementSize", UintegerValue(config.sensorMeasurementSize));

	factory.Set("RemoteAddress", Ipv4AddressValue (apNodeInterface.GetAddress(0)));
	factory.Set("RemotePort", UintegerValue (84));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable> ();

	for (uint16_t i = 0; i < config.Nsta; i++) {

		Ptr<Application> tcpClient = factory.Create<TCPSensorClient>();
		wifiStaNode.Get(i)->AddApplication(tcpClient);
		auto clientApp = ApplicationContainer(tcpClient);
		wireTCPClient(clientApp,i);

		double random = m_rv->GetValue(0, config.trafficInterval);
		clientApp.Start(MilliSeconds(0+random));
		clientApp.Stop(Seconds(config.simulationTime));
	}
}


void wireTCPServer(ApplicationContainer serverApp) {
	serverApp.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&tcpPacketReceivedAtServer));
	serverApp.Get(0)->TraceConnectWithoutContext("Retransmission", MakeCallback(&tcpRetransmissionAtServer));
	serverApp.Get(0)->TraceConnectWithoutContext("PacketDropped", MakeCallback(&tcpPacketDroppedAtServer));
	serverApp.Get(0)->TraceConnectWithoutContext("TCPStateChanged", MakeCallback(&tcpStateChangeAtServer));

	if(config.trafficType == "tcpipcamera") {
		serverApp.Get(0)->TraceConnectWithoutContext("DataReceived", MakeCallback(&tcpIPCameraDataReceivedAtServer));
	}
}

void wireTCPClient(ApplicationContainer clientApp, int i) {

	clientApp.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&NodeEntry::OnTcpPacketSent, nodes[i]));
	clientApp.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&NodeEntry::OnTcpEchoPacketReceived, nodes[i]));

	clientApp.Get(0)->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&NodeEntry::OnTcpCongestionWindowChanged, nodes[i]));
	clientApp.Get(0)->TraceConnectWithoutContext("RTO", MakeCallback(&NodeEntry::OnTcpRTOChanged, nodes[i]));
	clientApp.Get(0)->TraceConnectWithoutContext("RTT", MakeCallback(&NodeEntry::OnTcpRTTChanged, nodes[i]));
	clientApp.Get(0)->TraceConnectWithoutContext("SlowStartThreshold", MakeCallback(&NodeEntry::OnTcpSlowStartThresholdChanged, nodes[i]));
	clientApp.Get(0)->TraceConnectWithoutContext("EstimatedBW", MakeCallback(&NodeEntry::OnTcpEstimatedBWChanged, nodes[i]));

	clientApp.Get(0)->TraceConnectWithoutContext("TCPStateChanged", MakeCallback(&NodeEntry::OnTcpStateChanged, nodes[i]));
	clientApp.Get(0)->TraceConnectWithoutContext("Retransmission", MakeCallback(&NodeEntry::OnTcpRetransmission, nodes[i]));

	clientApp.Get(0)->TraceConnectWithoutContext("PacketDropped", MakeCallback(&NodeEntry::OnTcpPacketDropped, nodes[i]));

	if(config.trafficType == "tcpfirmware") {
		clientApp.Get(0)->TraceConnectWithoutContext("FirmwareUpdated", MakeCallback(&NodeEntry::OnTcpFirmwareUpdated, nodes[i]));
	}
	else if(config.trafficType == "tcpipcamera") {
	    clientApp.Get(0)->TraceConnectWithoutContext("DataSent", MakeCallback(&NodeEntry::OnTcpIPCameraDataSent, nodes[i]));
	    clientApp.Get(0)->TraceConnectWithoutContext("StreamStateChanged", MakeCallback(&NodeEntry::OnTcpIPCameraStreamStateChanged, nodes[i]));
	}
}

void configureTCPEchoClients() {
	TcpEchoClientHelper clientHelper(apNodeInterface.GetAddress(0), 80); //address of remote node
	clientHelper.SetAttribute("MaxPackets", UintegerValue(4294967295u));
	clientHelper.SetAttribute("Interval", TimeValue(MilliSeconds(config.trafficInterval)));
	//clientHelper.SetAttribute("IntervalDeviation", TimeValue(MilliSeconds(config.trafficIntervalDeviation)));
	clientHelper.SetAttribute("PacketSize", UintegerValue(config.payloadSize));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable> ();

	for (uint16_t i = 0; i < config.Nsta; i++) {
		ApplicationContainer clientApp = clientHelper.Install(wifiStaNode.Get(i));
		wireTCPClient(clientApp,i);

		double random = m_rv->GetValue(0, config.trafficInterval);
		clientApp.Start(MilliSeconds(0+random));
		//clientApp.Stop(Seconds(simulationTime + 1));
	}
}

void configureUDPClients() {
	        //Application start time
	        Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable> ();

	        UdpClientHelper myClient (apNodeInterface.GetAddress (0), 9); //address of remote node
	        myClient.SetAttribute ("MaxPackets", config.maxNumberOfPackets);
	        myClient.SetAttribute ("PacketSize", UintegerValue (config.payloadSize));
	          traffic_sta.clear ();
	          ifstream trafficfile (config.TrafficPath);
	           if (trafficfile.is_open())
	                 {
	                  uint16_t sta_id;
	                  float  sta_traffic;
	                  for (uint16_t kk=0; kk< config.Nsta; kk++)
	                    {
	                      trafficfile >> sta_id;
	                      trafficfile >> sta_traffic;
	                      traffic_sta.insert (std::make_pair(sta_id,sta_traffic)); //insert data
	                      //cout << "sta_id = " << sta_id << " sta_traffic = " << sta_traffic << "\n";
	                    }
	                  trafficfile.close();
	                }
	           else cout << "Unable to open file \n";

	          double randomStart = 0.0;
	          for (std::map<uint16_t,float>::iterator it=traffic_sta.begin(); it!=traffic_sta.end(); ++it)
	              {
	                  std::ostringstream intervalstr;
	                  intervalstr << (config.payloadSize*8)/(it->second * 1000000);
	                  std::string intervalsta = intervalstr.str();

	                  //config.trafficInterval = UintegerValue (Time (intervalsta));

	                  myClient.SetAttribute ("Interval", TimeValue (Time (intervalsta))); // TODO add to nodeEntry and visualize
	                  randomStart = m_rv->GetValue (0, (config.payloadSize*8)/(it->second * 1000000));
	                  ApplicationContainer clientApp = myClient.Install (wifiStaNode.Get(it->first));
	                  clientApp.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&NodeEntry::OnUdpPacketSent, nodes[it->first]));
	                  clientApp.Start (Seconds (1 + randomStart));
	                  //clientApp.Stop (Seconds (config.simulationTime+1)); //with this throughput is smaller
	               }
	              AppStartTime=Simulator::Now ().GetSeconds () + 1;
	              //Simulator::Stop (Seconds (config.simulationTime+1));
}

void configureUDPEchoClients() {
	UdpEchoClientHelper clientHelper(apNodeInterface.GetAddress(0), 9); //address of remote node
	clientHelper.SetAttribute("MaxPackets", UintegerValue(4294967295u));
	clientHelper.SetAttribute("Interval", TimeValue(MilliSeconds(config.trafficInterval)));
	//clientHelper.SetAttribute("IntervalDeviation", TimeValue(MilliSeconds(config.trafficIntervalDeviation)));
	clientHelper.SetAttribute("PacketSize", UintegerValue(config.payloadSize));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable> ();

	for (uint16_t i = 0; i < config.Nsta; i++) {
		ApplicationContainer clientApp = clientHelper.Install(wifiStaNode.Get(i));
		//clientApp.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&NodeEntry::OnUdpPacketSent, nodes[i]));
		//clientApp.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&NodeEntry::OnUdpEchoPacketReceived, nodes[i]));

		double random = m_rv->GetValue(0, config.trafficInterval);
		clientApp.Start(MilliSeconds(0+random));
		//clientApp.Stop(Seconds(simulationTime + 1));
	}
}


Time timeTx;
Time timeRx;
Time timeIdle;
Time timeSleep;

//ns3::int64x64_t throughouputArray [MaxSta];
Time timeIdleArray [MaxSta];
Time timeRxArray [MaxSta];
Time timeTxArray [MaxSta];
Time timeSleepArray [MaxSta];
double dist[MaxSta];

//it prints the information regarding the state of the device
void 
PhyStateTrace (std::string context, Time start, Time duration, enum WifiPhy::State state)
{   

    /*Get the number of the node from the context*/
    /*context = "/NodeList/"+strSTA+"/DeviceList/'*'/Phy/$ns3::YansWifiPhy/State/State"*/
    unsigned first = context.find("t/");
    unsigned last = context.find("/D");
    string strNew = context.substr ((first+2),(last-first-2));
    
    int node = std::stoi(strNew);

    //start calculating energy after complete association
    if  (GetAssocNum () == StaNum)
    {
        switch (state)
        {
            case WifiPhy::State::IDLE: //Idle
                timeIdle = timeIdle + duration;
                timeIdleArray[node] = timeIdleArray[node] + duration;
                break;
            case WifiPhy::State::RX: //Rx
                timeRx = timeRx + duration;
                timeRxArray[node] = timeRxArray[node] + duration;
                break;
            case WifiPhy::State::TX: //Tx
                timeTx = timeTx + duration;
                timeTxArray[node] = timeTxArray[node] + duration;
                break;
            case WifiPhy::State::SLEEP: //Sleep  
                timeSleep = timeSleep + duration;
                timeSleepArray[node] = timeSleepArray[node] + duration;
                break;        
        }
    }
}

int main (int argc, char *argv[])
{
  LogComponentEnable ("UdpServer", LOG_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_INFO);
  LogComponentEnable ("UdpEchoClientApplication", LOG_INFO);

  bool OutputPosition = true;
  config = Configuration(argc, argv);
    
    

  config.rps = configureRAW (config.rps, config.RAWConfigFile);
  config.Nsta = config.NRawSta;
    
    config.NSSFile = config.trafficType + "_" +
    std::to_string(config.Nsta) + "sta_"+
    std::to_string(config.NGroup) + "Group_" +
    std::to_string(config.NRawSlotNum)+ "slots_" +
    std::to_string(config.payloadSize) + "payload_" +
    std::to_string(config.totaltraffic) + "Mbps_" +
    std::to_string(config.BeaconInterval) + "BI"  + ".nss";

  stats = Statistics(config.Nsta);
  eventManager = SimulationEventManager(config.visualizerIP, config.visualizerPort, config.NSSFile);
  uint32_t totalRawGroups (0);
  for (int i = 0; i < config.rps.rpsset.size(); i++)
  {
	  int nRaw = config.rps.rpsset[i]->GetNumberOfRawGroups();
	  totalRawGroups += nRaw;
	  //cout << "Total raw groups after rps " << i << " is " << totalRawGroups << endl;
	  for (int j = 0; j < nRaw; j++){
		  config.totalRawSlots += config.rps.rpsset[i]->GetRawAssigmentObj(j).GetSlotNum();
		  //cout << "Total slots after group " << j << " is " << totalRawSlots << endl;
	  }

  }
  transmissionsPerTIMGroupAndSlotFromAPSinceLastInterval = vector<long>(config.totalRawSlots, 0);
  transmissionsPerTIMGroupAndSlotFromSTASinceLastInterval = vector<long>(config.totalRawSlots, 0);

  RngSeedManager::SetSeed (config.seed);

  wifiStaNode.Create (config.Nsta);
  wifiApNode.Create (1);

  YansWifiChannelHelper channelBuilder = YansWifiChannelHelper ();
  channelBuilder.AddPropagationLoss ("ns3::LogDistancePropagationLossModel","Exponent", DoubleValue(3.76) ,"ReferenceLoss", DoubleValue(8.0), "ReferenceDistance", DoubleValue(1.0));
  channelBuilder.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  Ptr<YansWifiChannel> channel = channelBuilder.Create();
  channel->TraceConnectWithoutContext("Transmission", MakeCallback(&onChannelTransmission)); //TODO

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetErrorRateModel ("ns3::YansErrorRateModel");
  phy.SetChannel (channel);
  phy.Set ("ShortGuardEnabled", BooleanValue (false));
  phy.Set ("ChannelWidth", UintegerValue (getBandwidth(config.DataMode))); // changed
  phy.Set ("EnergyDetectionThreshold", DoubleValue (-110.0));
  phy.Set ("CcaMode1Threshold", DoubleValue (-113.0));
  phy.Set ("TxGain", DoubleValue (0.0));
  phy.Set ("RxGain", DoubleValue (0.0));
  phy.Set ("TxPowerLevels", UintegerValue (1));
  phy.Set ("TxPowerEnd", DoubleValue (0.0));
  phy.Set ("TxPowerStart", DoubleValue (0.0));
  phy.Set ("RxNoiseFigure", DoubleValue (6.8));
  phy.Set ("LdpcEnabled", BooleanValue (true));
  phy.Set ("S1g1MfieldEnabled", BooleanValue (config.S1g1MfieldEnabled));

  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ah);
  S1gWifiMacHelper mac = S1gWifiMacHelper::Default ();



  Ssid ssid = Ssid ("ns380211ah");
  StringValue DataRate;
  DataRate = StringValue (getWifiMode(config.DataMode)); // changed

    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", DataRate, "ControlMode", DataRate);

  mac.SetType ("ns3::StaWifiMac",
                "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevice;
  staDevice = wifi.Install (phy, mac, wifiStaNode);

  mac.SetType ("ns3::ApWifiMac",
                 "Ssid", SsidValue (ssid),
                 "BeaconInterval", TimeValue (MicroSeconds(config.BeaconInterval)),
                 "NRawStations", UintegerValue (config.NRawSta),
                 "RPSsetup", RPSVectorValue (config.rps));

  phy.Set ("TxGain", DoubleValue (3.0));
  phy.Set ("RxGain", DoubleValue (3.0));
  phy.Set ("TxPowerLevels", UintegerValue (1));
  phy.Set ("TxPowerEnd", DoubleValue (30.0));
  phy.Set ("TxPowerStart", DoubleValue (30.0));
  phy.Set ("RxNoiseFigure", DoubleValue (6.8));

  apDevice = wifi.Install (phy, mac, wifiApNode);

  Config::Set ("/NodeList/*/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/BE_EdcaTxopN/Queue/MaxPacketNumber", UintegerValue(10));
  Config::Set ("/NodeList/*/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/BE_EdcaTxopN/Queue/MaxDelay", TimeValue (NanoSeconds (6000000000000)));

  std::ostringstream oss;
  oss << "/NodeList/"
		  << wifiApNode.Get(0)->GetId()
		  << "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::ApWifiMac/";
  Config::ConnectWithoutContext(oss.str () + "RpsIndex", MakeCallback (&RpsIndexTrace));
  Config::ConnectWithoutContext(oss.str () + "RawGroup", MakeCallback (&RawGroupTrace));
  Config::ConnectWithoutContext(oss.str () + "RawSlot", MakeCallback(&RawSlotTrace));


  // mobility.
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                       "X", StringValue ("1000.0"),
                                       "Y", StringValue ("1000.0"),
                                       "rho", StringValue (config.rho));
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiStaNode);


  MobilityHelper mobilityAp;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (1000.0, 1000.0, 0.0));
  mobilityAp.SetPositionAllocator (positionAlloc);
  mobilityAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityAp.Install(wifiApNode);


   /* Internet stack*/
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNode);

  Ipv4AddressHelper address;

  address.SetBase ("192.168.0.0", "255.255.0.0");
  //Ipv4InterfaceContainer staNodeInterface;
  //Ipv4InterfaceContainer apNodeInterface;

  staNodeInterface = address.Assign (staDevice);
  apNodeInterface = address.Assign (apDevice);

  //trace association
  for (uint16_t kk=0; kk< config.Nsta; kk++)
    {
      std::ostringstream STA;
      STA << kk;
      std::string strSTA = STA.str();

      assoc_record *m_assocrecord=new assoc_record;
      m_assocrecord->setstaid (kk);
      Config::Connect ("/NodeList/"+strSTA+"/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/Assoc", MakeCallback (&assoc_record::SetAssoc, m_assocrecord));
      Config::Connect ("/NodeList/"+strSTA+"/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/DeAssoc", MakeCallback (&assoc_record::UnsetAssoc, m_assocrecord));
      assoc_vector.push_back (m_assocrecord);
    }



    //Simulator::ScheduleNow(&UdpTraffic, config.Nsta, config.payloadSize, config.TrafficPath, staDevice);

    //Simulator::Schedule(Seconds(1), &CheckAssoc, config.Nsta, config.simulationTime, wifiApNode, wifiStaNode,apNodeInterface);


    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    PopulateArpCache ();


    // configure tracing for associations & other metrics
    configureNodes(wifiStaNode, staDevice);

	Config::Connect("/NodeList/" + std::to_string(config.Nsta) + "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyRxDropWithReason", MakeCallback(&OnAPPhyRxDrop));
	Config::Connect("/NodeList/" + std::to_string(config.Nsta) + "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::ApWifiMac/PacketToTransmitReceivedFromUpperLayer", MakeCallback(&OnAPPacketToTransmitReceived));

    Ptr<MobilityModel> mobility1 = wifiApNode.Get (0)->GetObject<MobilityModel>();
    Vector apposition = mobility1->GetPosition();
      if  (OutputPosition)
        {
          uint32_t i =0;
          while (i < config.Nsta)
            {
               Ptr<MobilityModel> mobility = wifiStaNode.Get (i)->GetObject<MobilityModel>();
               Vector position = mobility->GetPosition();
               nodes[i]->x = position.x;
               nodes[i]->y = position.y;
               std::cout << "Sta node#" << i << ", " << "position = " << position << std::endl;
               dist[i] = mobility -> GetDistanceFrom (wifiApNode.Get (0)->GetObject<MobilityModel>());
               i++;
            }
          std::cout << "AP node, position = " << apposition << std::endl;
        }
    
     /*Print of the state of the stations*/
        for (uint32_t i=0; i< config.Nsta; i++)
        {
            std::ostringstream STA;
            STA << i;
            std::string strSTA = STA.str();

            Config::Connect ("/NodeList/"+strSTA+"/DeviceList/*/Phy/$ns3::YansWifiPhy/State/State", MakeCallback(&PhyStateTrace));
        }

      eventManager.onStartHeader();
      eventManager.onStart(config);
      if (config.rps.rpsset.size() > 0)
    	  for (uint32_t i = 0; i < config.rps.rpsset.size(); i++)
    		  for (uint32_t j = 0; j < config.rps.rpsset[i]->GetNumberOfRawGroups(); j++)
    			  eventManager.onRawConfig(i, j, config.rps.rpsset[i]->GetRawAssigmentObj(j));

      for (uint32_t i = 0; i < config.Nsta; i++)
      	eventManager.onSTANodeCreated(*nodes[i]);
          
      eventManager.onAPNodeCreated(apposition.x, apposition.y);
      eventManager.onStatisticsHeader();

      sendStatistics(true);


      Simulator::Stop(Seconds(config.simulationTime + config.CoolDownPeriod)); // allow up to a minute after the client & server apps are finished to process the queue
      Simulator::Run ();

      // Visualizer throughput
      int pay=0, totalSuccessfulPackets=0, totalSentPackets=0;
      for (int i=0; i < config.Nsta; i++){
    	  totalSuccessfulPackets+= stats.get(i).NumberOfSuccessfulPackets;
    	  totalSentPackets+= stats.get(i).NumberOfSentPackets;
    	  pay+=stats.get(i).TotalPacketPayloadSize;
    	  cout << i << " sent: " << stats.get(i).NumberOfSentPackets << " ;succesfull: " << stats.get(i).NumberOfSuccessfulPackets << "; packetloss: "<< stats.get(i).GetPacketLoss(config.trafficType) << endl;
      }



        //Energy consumption per station
      timeRx = timeRx/config.Nsta;
      timeIdle = timeIdle/config.Nsta;
      timeTx = timeTx/config.Nsta;
      timeSleep = timeSleep/config.Nsta;  
      
    
        ofstream risultati;    
        //string addressresults = Outputpath + "/moreinfo.txt";
        string addressresults = config.OutputPath + "moreinfo.txt";
        risultati.open (addressresults.c_str(), ios::out | ios::trunc); 

        risultati << "Sta node#      distance        timerx      timeidle        timetx      timesleep      totenergy" << std::endl;           
        int i = 0;
        ns3::int64x64_t totenergycons = 0;
        string spazio = "         ";
        
          while (i < config.Nsta)
            {
              totenergycons = (timeRxArray[i].GetSeconds() * 4.4) + (timeIdleArray[i].GetSeconds() * 4.4) + (timeTxArray[i].GetSeconds() * 7.2);
               risultati << i << spazio << dist[i] << spazio << timeRxArray[i].GetSeconds() << spazio << timeIdleArray[i].GetSeconds() << spazio << timeTxArray[i].GetSeconds() << spazio << timeSleepArray[i].GetSeconds() << spazio << totenergycons << std::endl;

               totenergycons = 0;

/*
               cout << "================== Sleep " << stats.get(i).TotalSleepTime.GetSeconds() << endl;
               cout << "================== Tx " << stats.get(i).TotalTxTime.GetSeconds() << endl;
               cout << "================== Rx " << stats.get(i).TotalRxTime.GetSeconds() << endl;
               cout << "+++++++++++++++++++IDLE " << stats.get(i).TotalIdleTime.GetSeconds() << endl;
               cout << "ooooooooooooooooooo TOTENERGY " <<  stats.get(i).GetTotalEnergyConsumption() << " mW" << endl;
               cout << "Rx+Idle ENERGY " <<  stats.get(i).EnergyRxIdle << " mW" << endl;
               cout << "Tx ENERGY " <<  stats.get(i).EnergyTx << " mW" << endl;*/

               i++;
            }

          risultati.close();

		
      if (config.trafficType == "udp")
      {
          double throughput = 0;
          uint32_t totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
          throughput = totalPacketsThrough * config.payloadSize * 8 / (config.simulationTime * 1000000.0);
          cout << "totalPacketsThrough " << totalPacketsThrough << " ++my " << totalSuccessfulPackets << endl;
          cout << "throughput " << throughput << " ++my " << pay*8./(config.simulationTime * 1000000.0) << endl;
          std::cout << "datarate" << "\t" << "throughput" << std::endl;
          std::cout << config.datarate << "\t" << throughput << " Mbit/s" << std::endl;

      }
      cout << "total packet loss " << 100 - 100. * totalSuccessfulPackets/totalSentPackets<<  endl;
      Simulator::Destroy ();

      return 0;
}
