

struct GeoLocation{
	float latitude;
	float longitue;
}

class GeoTracker{

	static GeoTracker instance;

	public:
		static GeoTracker getInstance();
		GeoLocation getCurrentLocation();			
}
