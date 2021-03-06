#include "geo.h"
#include "wpawrapper.h"
#include "pcapwifi.h"


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <stdio.h>
#include <pcap.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h> 
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <mutex>
#include <iterator>

using namespace std;
using namespace rapidjson;

const char* SAVED_NETWORK_JSON_PATH = "./saved-network.json";
const int MAX_RANGE = 50;
pcap_t* pcap = NULL;
ofstream reportFile;
mutex syncMutex;

struct timeval event;

typedef void (*pcap_packet_handler)(u_char* args, const struct pcap_pkthdr* header, const u_char* packet);

int wpa_service(const WifiInterface* interface){
	
	//start wpa_supplicant, not as daemon
	string command = "wpa_supplicant";
	string params[4];
	params[0] = "";
	params[1] = "-i" + interface->getName();
	params[2] = "-C/var/run/wpa_supplicant";
	
	cout<<"Runing wpa daemon on "<<interface->getName()<<endl;
	int result;
	result = execl("/sbin/wpa_supplicant", "wpa_suppplicant", params[0].c_str(), params[1].c_str(), params[2].c_str(), (void*)0);		
	
	//only return if failed
	return result;
}

string print_hex_ascii_line(const u_char* payload, int len){

	int i;
	int gap;
	const u_char* ch;
	
	char output[1024]="";
	
	cout<<"length: "<<len<<endl;
	
	/* hex */
	ch = payload;
	for(i = 0; i < len; i++) {
		printf("%02x ", *ch);
		ch++;
		/* print extra space after 8th byte for visual aid */
		if (i == 7)
			printf(" ");
	}
	/* print space to handle line less than 8 bytes */
	if (len < 8)
		printf(" ");
	
	/* fill hex gap with spaces if not full line */
	if (len < 16) {
		gap = 16 - len;
		for (i = 0; i < gap; i++) {
			printf("   ");
		}
	}
	printf("   ");
	
	/* ascii (if printable) */
	ch = payload;
	for(i = 0; i < len; i++) {
		if (isprint(*ch))
			printf("%c", *ch);
		else
			printf(".");
		ch++;
	}

	printf("\n");

	return string(output);
}

void displayMAC(const unsigned char* mac){
	char macdis[20];
	mac_addr_n2a(macdis, mac);
	printf("mac: %s\n", macdis);
}

bool containString(vector<u_char*> vector, const u_char* str){
	bool found = false;
	for (const u_char* i : vector){
		//cout<<"vector: ";
		//displayMAC(i);
		if (memcmp(i, str, ETH_ALEN) == 0){
			break;
			found = true;
		}
	}
	/*if (!found){
		//try again with ETH_ALEN/2
		for (const u_char* i : vector){
			cout<<"vector: ";
			//displayMAC(i);
			if (memcmp(i, str, ETH_ALEN/2) == 0){			
				found = true;
				break;
			}
		}
	}*/
	return found;
}

void report_to_file(ofstream& file, const char* des, struct timeval& time){
	cout<<"EVENT: "<<des<<endl;
	if (event.tv_sec != 0){
		if (time.tv_usec > event.tv_usec){
			if (time.tv_sec-event.tv_sec>0) 
				file<<time.tv_sec-event.tv_sec<<"";
			file<<setfill('0')<<setw(3)<<(time.tv_usec-event.tv_usec)/1000<<" ms"<<endl;
		}
		else{
			if (time.tv_sec-event.tv_sec-1>0) 
				file<<time.tv_sec-event.tv_sec-1<<"";
			file<<setfill('0')<<setw(3)<<(time.tv_usec-event.tv_usec+1000000)/1000<<" ms"<<endl;
		}				
		
	}
	file<<des<<endl;
	
	event.tv_usec = time.tv_usec;
	event.tv_sec = time.tv_sec;
}

int packet_sniffing(WifiInterface* interface, WifiNetwork* network){
	

	
	char errbuf[PCAP_ERRBUF_SIZE] = "";
	int ret;
	bool loop = true;
	event.tv_sec = 0;
	event.tv_usec = 0;
	
	//--1. find the first probe response of current network
	bool firstProbeResponse = false;
	//--2. find the authen request
	bool authRequest = false;			
	//--3. find the authen response
	bool authResponse = false;
	//--4. find the asso/reasso request
	bool assoRequest = false;
	//--5. find the ass/reassp response
	bool assoResponse = false;
	//--6. if WPA: find the first and last key exchanging packet EAPOL		
	//	   if EAP: find the first and last data packet EAPOL
	int counterEAP = 0;
	bool firstEAP = false;
	bool lastEAP = false;
	//--7. find the first QoS data packet
	bool firstData = false;
	
	const char* devName = (interface->getName()+string("mon")).c_str();
	//filter strategy
	//--1: coming in and out interface (MAC address) wlan addr1 ehost (receiver) or wlan addr2 ehost (sender)
	vector<unsigned char*>ap_macs;
	string mac = interface->getDevice()->getDisplayableMacAddress();
	string filter = string("wlan addr1 ") + mac + string(" or wlan addr2 ") + mac;
  
	struct bpf_program fp;	
 	bpf_u_int32 mask= PCAP_NETMASK_UNKNOWN ;
 	
	cout<<"Sniffing on interface: "<<devName<<", filter: "<<filter<<endl;

  	pcap = pcap_open_live(devName, BUFSIZ, 1, 1000, errbuf);
  	if (pcap == NULL) {
    	cout<<"Cannot initialize "<<devName<<" because: "<<errbuf<<endl;
    	syncMutex.unlock();
		return -1;
	}	
    
	if (pcap_compile(pcap, &fp, filter.c_str(), 1, mask) == -1) {
		cout<<"Couldn't parse filter"<<endl;
		syncMutex.unlock();
		return -1;
	}
	
	if (pcap_setfilter(pcap, &fp) == -1) {
		cout<<"Couldn't install filter"<<endl;
		syncMutex.unlock();
		return -1;
	}
		
	cout<<"Starting pcap loop"<<endl;
	
	reportFile.open(string("./") + interface->getName() + string(".report"), fstream::app);
	reportFile<<"-----------------WIFI EVENT REPORT------------------"<<endl;
	reportFile<<"Interface: "<<interface->getName()<<" - network: "<<network->getSSID()<<endl;
	syncMutex.unlock();
		
	
	while (loop){
		const u_char* packet = NULL;
		struct pcap_pkthdr packetHeader;
		
		//force reading packet		
		while (packet == NULL)
			packet = pcap_next(pcap, &packetHeader);
								
		struct radiotap_frame* radiotap_hdr;
		radiotap_hdr = (struct radiotap_frame*)packet;

		struct ieee80211_frame* ieee80211_hdr;
		ieee80211_hdr = (struct ieee80211_frame*)(packet + radiotap_hdr->header_len);			
			
		enum IEEE80211_FRAME_TYPE frameType = get80211FrameType(ieee80211_hdr);
		enum IEEE80211_FRAME_SUBTYPE frameSubType = get80211FrameSubType(ieee80211_hdr, frameType);
	
		struct ieee80211_management_frame* mgnt_frame;
		
		cout<<"frame "<<packetHeader.caplen;		
		switch (frameType) {
			case MANAGEMENT_FRAME:
				mgnt_frame = (struct ieee80211_management_frame*)(packet + radiotap_hdr->header_len + MANAGEMENT_FRAME_HDRLEN);
				if (frameSubType == PROBE_RESPONSE_FRAME){											
					//check if this packet came from the desired AP
					cout<<" - probe response\n";
					char ssid[100] = "";
					getTaggedValue(mgnt_frame, TAGGED_SSID, packetHeader.caplen - radiotap_hdr->header_len - MANAGEMENT_FRAME_HDRLEN, ssid);
					
					//cout<<" -- captured ssid: "<<ssid<<endl;
					//cout<<" -- network ssid: "<<network->getSSID().c_str()<<endl;
					if (ssid!=NULL && strcmp(ssid, network->getSSID().c_str())==0){
						if (!firstProbeResponse){
							firstProbeResponse = true;
							report_to_file(reportFile, "First probe response", packetHeader.ts);
						}
						u_char* mac = (u_char*)malloc(ETH_ALEN);
						memcpy(mac, ieee80211_hdr->sender, ETH_ALEN);
						ap_macs.push_back(mac);							
					}
				}
				else if (frameSubType == AUTHENTICATION_FRAME){
					cout<<" - authen\n";															
					if (memcmp(ieee80211_hdr->sender, interface->getDevice()->getMacAddress(), ETH_ALEN) == 0 /*&& (containString(ap_macs, ieee80211_hdr->receiver))*/){
						//authentication request
						if (!authRequest){
							authRequest = true;
							report_to_file(reportFile, "Authentication request", packetHeader.ts);							
						}								
					}
					else if (memcmp(ieee80211_hdr->receiver, interface->getDevice()->getMacAddress(), ETH_ALEN) == 0 /*&& (ap_macs.size() ==0 || containString(ap_macs, ieee80211_hdr->sender))*/){
						if (!authResponse){
							authResponse = true;
							report_to_file(reportFile, "Authentication response", packetHeader.ts);	
						}	
					}			
				}
				else if (frameSubType == ASSOCIATION_REQUEST_FRAME){
					cout<<" - asso\n";
					if (memcmp(ieee80211_hdr->sender, interface->getDevice()->getMacAddress(), ETH_ALEN) == 0 /*&& (ap_macs.size() ==0 || containString(ap_macs, ieee80211_hdr->receiver))*/){
						//association request
						if (!assoRequest){								
							assoRequest = true;
							report_to_file(reportFile, "Association request", packetHeader.ts);	
						}								
					}
				}
				else if (frameSubType == ASSOCIATION_RESPONSE_FRAME){
					if (memcmp(ieee80211_hdr->receiver, interface->getDevice()->getMacAddress(), ETH_ALEN) == 0 /*&& (ap_macs.size() ==0 || containString(ap_macs, ieee80211_hdr->sender))*/){
						if (!assoResponse){								
							assoResponse = true;
							report_to_file(reportFile, "Association response", packetHeader.ts);								
						}
					}
				}									
				break;
				
			case DATA_FRAME:
				cout<<" - data\n";			
				if (lastEAP){
					if (!firstData){
						firstData = true;
						report_to_file(reportFile, "First data packet", packetHeader.ts);
						loop = false;
					}
				}
				else
				{
					if (frameSubType == QOS_DATA_FRAME){					
						struct ieee80211_data_llc_frame* llc_hdr = (struct ieee80211_data_llc_frame*)(packet + radiotap_hdr->header_len + QoSHeaderLen(ieee80211_hdr));
						if (llc_hdr->type == IEEE_8021X_AUTHENTICATION){
							if (!firstEAP){
								firstEAP = true;
								report_to_file(reportFile, "First EAP", packetHeader.ts);	
							}
							
							//count 4 ways handshake packets
							struct ieee8021X_authentication_frame* eap_auth = (struct ieee8021X_authentication_frame*)(packet + radiotap_hdr->header_len + QoSHeaderLen(ieee80211_hdr) + LLC_HEADER_LEN);
							if (eap_auth->type == IEEE_8021X_KEY_FRAME){
								counterEAP++;
							}
							if (counterEAP == 4){									
								lastEAP = true;
								report_to_file(reportFile, "Last EAP", packetHeader.ts);
								
								//call dhclient											
								string dhclient = string("dhclient ") + interface->getName();
								system(dhclient.c_str());								
							}
						}
					}
				}					
				break;
				
			default:
				break;
		}
	}
	reportFile.close();

	return 0;
}


int report(){

	bool fixed_freq = true;

	WifiController* netController = new WifiController();
	vector<WifiInterface*> interfaces = netController->getNetworkInterfaces();
<<<<<<< HEAD

	//check for wpa_supplicant
	//int ret;
	/*ret = system("wpa_supplicant");
	if (ret != 0){
		cout<<"Error: wpa_supplicant is not installed. Try: sudo apt-get install wpasupplicant"<<endl;
		return -1;
	}
	ret = system("ifconfig");
	if (ret != 0){
		cout<<"Error: ifconfig not found on this system"<<endl;
		return -1;
	}*/	

	cout<<"Killing disturbing processes..."<<endl;
	//manually kill network-manager
	system("service network-manager stop");	
	system("service wpa_supplicant stop");		
	
	//run one wpa_supplicant for each interface
	int pCount = 0;
	pid_t pid = getpid();
	int cpid[interfaces.size()]; //child processes's id
	
	//fill zero to children process ids
	memset(cpid, 0, sizeof(cpid));
	
	vector<WifiInterface*> virtualInterfaces;
	
	for (WifiInterface* i : interfaces){
		cout<<"Interface: "<<i->getName()<<", MAC: "<<i->getDevice()->getDisplayableMacAddress()<<endl;
		if (pid != 0){									
			//only call fork in parent process
			pid = fork();
			if (pid == 0){
				//starting wpa process for wifi handling
				return wpa_service(i);
			}
			else{
				//this is parent process. save the pid for later closing.
				//cout<<"Starting pid: "<<pid<<" as wpa process..."<<endl;
				cpid[pCount]=pid;
				pCount++;
				
				//create new virtual interface and enable monitor mode
				//cout<<"Creating new virtual interface: "<<i->getName() + string("mon")<<endl;
				WifiInterface* vi = i->getDevice()->addVirtualInterface(i, i->getName() + string("mon"), NL80211_IFTYPE_MONITOR);
				
				virtualInterfaces.push_back(vi);
				//bring up this virtual interface
				string bringupcommand;
				//bringupcommand = string("ifconfig ") + i->getName() + string(" up");
				cout<<"Bringing up: "<<bringupcommand<<endl;
				system(bringupcommand.c_str());				
				
				bringupcommand = string("ifconfig ") + i->getName() + string("mon") + string(" up");
				cout<<"Bringing up: "<<bringupcommand<<endl;
				system(bringupcommand.c_str());								
			}
		}
		else{
			//this is the next loop inside child proccess with pid overriden to 0 after a success fork
			//just break to stop the redundancy iteration.
			break;
		}
	}
	
	//parent does this
	if (pid!=0){
	
		//wait for wireshark
		string ok;
		cout<<"start wireshark...";
		cin>>ok;
	
		//1. load saved network
		ifstream file(SAVED_NETWORK_JSON_PATH);
		string contents = string(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
		Document doc;
		doc.Parse(contents.c_str());	
		
		//2. get current location
		GeoTracker* geoTracker = GeoTracker::getInstance();
		struct GeoLocation* currentLocation = geoTracker->getCurrentLocation();
		//std::cout<<"Current location: "<<currentLocation->latitude<<", "<<currentLocation->longitude<<std::endl;		

		//4. iterate through access points and check for distance
		Value &networks = doc["networks"];
		if (networks.IsNull() || !networks.IsArray()){
			cout<<"Error: Bad saved network file!"<<endl;
			return -1;		
		}
		else{
			//test with each network interface							
			for (SizeType i=0; i<networks.Size(); i++){
				Value& net = networks[i];			
				if (!net.IsNull()){
					if (net["enabled"].GetBool()){
					
						Value& aps = net["aps"];
						Value& encryption = net["encryption"];
						
						string cipherMode = encryption["key_mgmt"].GetString();
						string networkName = net["ssid"].GetString();
						cout<<"Into network: "<<networkName<<endl;
						cout<<"--cipher: "<<cipherMode<<endl;

						string request;
						string response;
						string networkID;

						if (!aps.IsNull() && aps.IsArray()){
							for (SizeType j=0; j<aps.Size(); j++){
								//4.1 iterate through access point to get those in range
								Value& ap = aps[j];
								if (!ap["lat"].IsNull() && !ap["long"].IsNull()){

									GeoLocation* apLocation = new GeoLocation(ap["lat"].GetFloat(), ap["long"].GetFloat());
									float distance = geoTracker->getDistance(apLocation, currentLocation);
									if (distance <= MAX_RANGE){
										//try to connect by each interface
										for (WifiInterface* interface : interfaces){
											//create control wrapper
											WpaControlWrapper* wpaControl = new WpaControlWrapper(interface);
											//string ok;
											//cout<<"wait for wpa_cli"<<endl;
											//cin>>ok;
											
											if (fixed_freq){
												request = string("SET freq_list ") + to_string(ap["frequency"].GetInt());
												wpaControl->request(request, response);
											}
											
											//set AP scan
											request = string("SET ap_scan 1");
											wpaControl->request(request, response);	
	
											//add network, response contains network ID
											request = string("ADD_NETWORK");
											wpaControl->request(request, response);									
											networkID = string(response);									
									
											//set network ssid
											request = string("SET_NETWORK ") + response;
											wpaControl->request(request + string(" ssid \"") + networkName + string("\""), response);
											
											//set cipher parameters
											for (Value::MemberIterator m = encryption.MemberBegin(); m!= encryption.MemberEnd(); m++){
    											wpaControl->request(request + string(" ") + m->name.GetString() + string(" ") + m->value.GetString(), response);
    											cout<<m->name.GetString()<<endl;
    										}

											//start sniffing process
											WifiNetwork* wifiNetwork = new WifiNetwork(networkName);
										
											syncMutex.lock();
											//this thread will unlock the mutex
											thread sniffing(packet_sniffing, interface, wifiNetwork);
										
											syncMutex.lock(); //hang here until sniffing thread ready to enter its loop										
											//enable this network then wait for result
											request = string("ENABLE_NETWORK ") + networkID;
											wpaControl->request(request, response);
											syncMutex.unlock();
										
											//join the sniffing thread
											sniffing.join();
											cout<<"In main: sniffing thread terminated\n";
											
											
											cout<<"wait before disable network"<<endl;
											//cin>>ok;

											//disable this network on this interface
											request = string("DISABLE_NETWORK ") + networkID;
											wpaControl->request(request, response);			
											request = string("REMOVE_NETWORK ") + networkID;
											wpaControl->request(request, response);														
										}
										//at least one ap of this network found, break
										break;
									}							
								}
							}
						}
					}							
				}				
			}
		}

		//kill children wpa processes
		for (int i : cpid){
			if (i>0){
				cout<<"Sending sigkill to "<<i<<endl;
				kill(i, SIGKILL);
			}
		}
		
		//wait for children processes to terminate
		int status;
		while (pCount>0){
			pid = wait(&status);
			if (pid>0){
				cout<<"Process id: "<<pid<<" returned"<<endl;
			}
			else{
				cout<<"Child process returned with error"<<endl;
			}
			pCount--;
		}
		
		//remove virtual device
		for (WifiInterface* vi : virtualInterfaces){
			cout<<"Removing vitual interface: "<<vi->getName()<<endl;
			vi->getDevice()->removeVirtualInterface(vi);
		}				
		
		//restart network-manager
		//system("service network-manager start");
		return EXIT_SUCCESS;
	}
}

int main(int argc, char** argv){
	return report();
}









