#include "network.h"
#include "geo.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

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

int run_report_service(const WifiInterface* interface){
	
	
	//start wpa_supplicant, not as daemon
	string command = "wpa_supplicant";
	string params[4];
	params[0] = "";
	params[1] = "-i" + interface->getName();
	params[2] = "-C/var/run/wpa_supplicant";
	
	cout<<"Runing wpa daemon on "<<interface->getName()<<endl;
	int result;
	//int result = execl("/sbin/wpa_supplicant", "wpa_suppplicant", params[0].c_str(), params[1].c_str(), params[2].c_str(), (void*)0);		
	
	return result;
}

int report(){


	WifiController* netController = new WifiController();
	vector<WifiInterface*> interfacesOriginal;
	//check for airmon-ng
	int ret = system("airmon-ng");
	if (ret != 0){
		cout<<"Error: airmon-ng is not installed. try sudo apt-get install aircrack-ng"<<endl;
		return -1;
	}
	else{
		//kill all disturbing process
		cout<<"Killing disturbing processes"<<endl;
		//manually kill network-manager
		//system("service network-manager stop");
		//system("service avahi-daemon stop");
		system("airmon-ng check kill");
		
		interfacesOriginal = netController->getNetworkInterfaces();	
		for (WifiInterface* i : interfacesOriginal){
			cout<<"Starting airmon-ng on "<<i->getName()<<endl;;
			string command = string("airmon-ng start ") + i->getName();
			int ret = system(command.c_str());
			if (ret < 0){
				cout<<"Error: cannot start monitoring mode on"<<i->getName()<<endl;
			}
		}
	}
	
	//get network interfaces that had monitoring mode enabled
	vector<WifiInterface*>::iterator ite;	
	vector<WifiInterface*> interfaces = netController->getNetworkInterfaces();
	
	for (ite = interfaces.begin(); ite < interfaces.end(); ite++){
		if ((*ite)->getName().find("mon") == string::npos){
			//cannot start airmon-ng on this interface
			cout<<"Removing "<<(*ite)->getName()<<" for not supporting airmon"<<endl;
			ite = interfaces.erase(ite);
		}
	}		
	
	//run one wpa_supplicant for each interface
	int pCount = 0;
	pid_t pid = getpid();
	int cpid[interfaces.size()]; //child processes's id
	
	//fill zero to children process ids
	memset(cpid, 0, sizeof(pid));
	
	for (WifiInterface* i : interfaces){
		if (pid != 0){						
			
			//only call fork in parent process
			pid = fork();
			if (pid == 0){
				//this is child process
				return run_report_service(i);
			}
			else{
				//this is parent process. save the pid for later closing.
				cout<<"Starting pid: "<<pid<<" as child process..."<<endl;
				cpid[pCount]=pid;
				pCount++;
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
	}
	
	//check mode
	int i=100;
	while (i>0){
		system("iwconfig");
		i--;
	}
	
	
	//stop airmon-ng
	for (WifiInterface* interface : interfaces){
		string command = string("airmon-ng stop ") + interface->getName();
		cout<<"stop airmon on "<<interface->getName()<<endl;
		system(command.c_str());
	}
	
	//kill children wpa processes
	for (int i : cpid){
		cout<<"Sending sigkill to "<<i<<endl;
		kill(i, SIGKILL);
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
	
	return EXIT_SUCCESS;

}

int main(int argc, char** argv){
	return report();
}









