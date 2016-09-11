#include "geo.h"

GeoLocation::GeoLocation(float latitude, float longitude){
	this->latitude = latitude;
	this->longitude = longitude;
}

GeoTracker* GeoTracker::instance = new GeoTracker();

GeoTracker::GeoTracker(){
}

GeoTracker* GeoTracker::getInstance(){
	return GeoTracker::instance;
}

struct GeoLocation* GeoTracker::getCurrentLocation(){
	struct GeoLocation* currentLocation = new GeoLocation();
	currentLocation->latitude = 0;
	currentLocation->longitude = 0;
	return currentLocation;
}

float GeoTracker::getDistance(const GeoLocation* l1, const GeoLocation* l2){
	return sqrt((l1->latitude-l2->latitude)*(l1->latitude-l2->latitude)+(l1->longitude-l2->longitude)*(l1->longitude-l2->longitude));
}
