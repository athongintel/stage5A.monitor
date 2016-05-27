//#include "network.h"
#include "geo.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

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

void report(){
	
	//1. load saved network
	ifstream file(SAVED_NETWORK_JSON_PATH);
	string contents = string(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
	Document doc;
	doc.Parse(contents.c_str());
	
	//2. get current location
	GeoTracker* geoTracker = GeoTracker::getInstance();
	struct GeoLocation* currentLocation = geoTracker->getCurrentLocation();
	//std::cout<<"Current location: "<<currentLocation->latitude<<", "<<currentLocation->longitude<<std::endl;
	
	//3. launch packet capturing and analysing module
	
	//4. iterate through access points and check for distance
	Value &networks = doc["networks"];
	if (networks.IsNull() || !networks.IsArray()){
		cout<<"Error: Bad saved network file!"<<endl;
		return;		
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
								cout<<"got this ap:"<<ap["mac"].GetString()<<endl;							
							}
						}
					}
				}
			}
		}
	}

}

int main(int argc, char** argv){
	report();
}









