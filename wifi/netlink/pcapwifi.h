
#ifndef ETH_ALEN
	#define ETH_ALEN 6
#endif

#ifndef _IEEE_80211_FRAME_TYPE_
#define _IEEE_80211_FRAME_TYPE_
		
	struct pcap_radiotap{
		u_char header_revision;
		u_char header_pad;
		u_short header_len;
		u_char header;
	};

	struct pcap_ieee80211{
		u_short frame_control;
		u_short duration;
		u_char receiver[ETH_ALEN];
		u_char sender[ETH_ALEN];
	};
	
#endif

enum 




