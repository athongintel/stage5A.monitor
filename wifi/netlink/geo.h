

struct GeoLocation{
	float lattitude;
	float longitude;
}

class GeoTracker{

	static struct GeoTracker* instance = new GeoTracker();

	GeoTracker();

	public:
		static GeoTracker getInstance();
		struct* GeoLocation getCurrentLocation();			
}
