#include "network.h"
#include "geo.h"

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
#include <fstream>

using namespace std;
using namespace rapidjson;

/*void test_connect(){
	WifiController* netController = new WifiController();
	vector<WifiInterface*> interfaces = netController->getNetworkInterfaces();
	vector<WifiNetwork*> networks;
	//int networkcount=0;
	int networkcount;
	int apcount;
	for (auto &i : interfaces){
		cout<<i->getName()<<" "<<i->getDisplayableMacAddress()<<endl;
		cout<<"I'm performing a full scan..."<<endl;
		networks = i->fullNetworkScan();	
		for (auto &net : networks){
			cout<<networkcount<<": "<<(net->SSID.empty()? "Hidden network" : net->SSID)<<endl;
			networkcount++;
			apcount=0;
			/*if (strcmp(net->SSID, "(Aix*Marseille universite")==0){
				cout<<"Trying to connect to "<<net->SSID<<" access point..."<<endl;				
			}
			bool connect = false;
			for (auto &ap : net->accessPoints){
				cout<<" -- "<<apcount<<":  "<<ap->getDisplayableBSSID()<<"  "<<ap->getFrequency()<<"  "<<endl;
				apcount++;
				/*if (!connect){
					//connect to first access point
					connect = true;
					i->connect(ap);
				}
			}
		}
		i->disconnect();
		cout<<"Disconnected!"<<endl;
	}
	int selectedInterface;
	int selectedNetwork;
	int selectedAP;
	cout<<"Selected interface to connect: ";
	cin>>selectedInterface;
	cout<<"Which network do you want connect: ";
	cin>>selectedNetwork;
	cout<<"Which AP of "<<networks[selectedNetwork]->SSID<<" you want to connect: ";
	cin>>selectedAP;
	
	
	//connect to this ap
	cout<<"Trying to connect to "<<networks[selectedNetwork]->accessPoints[selectedAP]->getDisplayableBSSID()<<" of "<<networks[selectedNetwork]->SSID<<" on "<<interfaces[selectedInterface]->getName()<<endl;
	
	interfaces[selectedInterface]->connect(networks[selectedNetwork]->accessPoints[selectedAP]);	

}*/

const char* SAVED_NETWORK_JSON_PATH = "./saved-network.json";
const int MAX_RANGE = 50;

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

void wifi_connection_analyser(u_char* args, const struct pcap_pkthdr* header, const u_char* packet){
	cout<<"yeah, I can enter here!!"<<endl;
}

int packet_sniffing(WifiInterface* interface, string filter, pcap_packet_handler handler){
	
	pcap_t* pcap = NULL;
	char errbuf[PCAP_ERRBUF_SIZE];
	int res;
	
	const char* devName = "wlan0mon";
  
	struct bpf_program fp;	
	char * filter_exp;
 	bpf_u_int32 mask= PCAP_NETMASK_UNKNOWN ;
 	
	cout<<"Sniffing on interface:"<<devName<<endl;

	try{
  	pcap = pcap_open_live(devName, BUFSIZ, 1, 1000, errbuf);
  	cout<<"no error pcap open"<<endl;
  	}
  	catch (exception& e)
  	{
  	cout<<"exception: "<<e.what()<<endl;
  	}
  	
  	cout<<"error here 1"<<endl;
  	if (pcap == NULL) {
    	cout<<"Cannot initialize "<<devName<<" because: "<<errbuf<<endl;
    	return(-1);
	}
    
    cout<<"error here 3"<<endl;
	if (pcap_compile(pcap, &fp, filter.c_str(), 0, mask) == -1) {
		cout<<"Couldn't parse filter"<<endl;
		return(-1);
	}
	
	cout<<"error here 4"<<endl;
	if (pcap_setfilter(pcap, &fp) == -1) {
		cout<<"Couldn't install filter"<<endl;
		return(-1);
	}

	cout<<"starting pcap loop"<<endl;
	res = pcap_loop(pcap, 0, handler, NULL);
	return(res);
}


int report(){

	WifiController* netController = new WifiController();
	vector<WifiInterface*> interfaces = netController->getNetworkInterfaces();

	//check for wpa_supplicant
	int ret;
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
	int cpid[interfaces.size()*2]; //child processes's id
	
	//fill zero to children process ids
	memset(cpid, 0, sizeof(cpid));
	
	for (WifiInterface* i : interfaces){
		if (pid != 0){									
			//only call fork in parent process
			pid = fork();
			if (pid == 0){
				//this is child process
				return wpa_service(i);
			}
			else{
				//this is parent process. save the pid for later closing.
				cout<<"Starting pid: "<<pid<<" as wpa process..."<<endl;
				cpid[pCount]=pid;
				pCount++;
				
				//create new virtual interface and enable monitor mode
				cout<<"Creating new virtual interface: "<<i->getName() + string("mon")<<endl;
				WifiInterface* virtualInterface = i->getDevice()->addVirtualInterface(i, i->getName() + string("mon"), NL80211_IFTYPE_MONITOR);
				//bring up this virtual interface
				string bringupcommand;
				bringupcommand = string("ifconfig ") + i->getName() + string(" up");
				cout<<"Bringing up: "<<bringupcommand<<endl;
				system(bringupcommand.c_str());				
				
				bringupcommand = string("ifconfig ") + i->getName() + string("mon") + string(" up");
				cout<<"Bringing up: "<<bringupcommand<<endl;
				system(bringupcommand.c_str());
								
				//start packet sniffing on the virtual interface
				pid = fork();
				if (pid == 0){
					//child sniffing process
					string filter = "";
					cout<<"entering snipping process"<<endl;
					return packet_sniffing(virtualInterface, filter, wifi_connection_analyser);
				}
				else{
					cout<<"Starting pid: "<<pid<<" as sniffing process..."<<endl;
					cpid[pCount]=pid;
					pCount++;
				}
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
			for (SizeType i=0; i<networks.Size(); i++){
				Value& net = networks[i];			
				if (!net.IsNull()){
					Value& aps = net["aps"];
					cout<<"Into network: "<<net["ssid"].GetString()<<endl;
					if (!aps.IsNull() && aps.IsArray()){
						for (SizeType j=0; j<aps.Size(); j++){
							//4.1 iterate through access point to get those in range
							Value& ap = aps[j];
							if (!ap["lat"].IsNull() && !ap["long"].IsNull()){
								GeoLocation* apLocation = new GeoLocation(ap["lat"].GetFloat(), ap["long"].GetFloat());
								float distance = geoTracker->getDistance(apLocation, currentLocation);
								if (distance <= MAX_RANGE){
									//4.2 try to connect to this access point
									//cout<<"got this ap:"<<ap["mac"].GetString()<<endl;					
								}
							}
						}
					}
				}
			}
		}

		//kill children wpa processes
		/*for (int i : cpid){
			if (i>0){
				cout<<"Sending sigkill to "<<i<<endl;
				kill(i, SIGKILL);
			}
		}*/
	
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
	
		return EXIT_SUCCESS;
	}

}

int main(int argc, char** argv){
	return report();
}









