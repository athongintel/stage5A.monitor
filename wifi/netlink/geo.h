#include <math.h>

class GeoLocation{
	friend class GeoTracker;
	
	float latitude;
	float longitude;
	public:			
		GeoLocation(float latitude = 0, float longitude = 0);
	
};

class GeoTracker{

	static struct GeoTracker* instance;

	GeoTracker();

	public:
		static GeoTracker* getInstance();
		GeoLocation* getCurrentLocation();
		float getDistance(const GeoLocation* l1, const GeoLocation* l2);		
};
