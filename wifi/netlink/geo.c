GeoTracer* GeoTracker::getInstance(){
	return GeoTracer::instance();
}

struct GeoLocation* GeoTracker::getCurrentLocation(){
	struct GeoLocation* currentLocation = new GeoLocation();
	currentLocation->latitude = 0;
	currentLocation->longitude = 0;
	return currentLocation;
}
