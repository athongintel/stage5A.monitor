
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifndef ETH_ALEN
	#define ETH_ALEN 6
#endif

#ifndef _IEEE_80211_FRAME_TYPE_
#define _IEEE_80211_FRAME_TYPE_

	struct radiotap_frame{
		u_char header_revision;
		u_char header_pad;
		u_short header_len;
		u_char header;
	};

	struct ieee80211_frame{
		u_short frame_control;
		u_short duration;
		u_char receiver[ETH_ALEN];
		u_char sender[ETH_ALEN];
	};
	
	struct ieee80211_management_frame{
		u_char fixedParameters[12];
		u_char taggedParameters;
	};
	
	struct ieee80211_data_llc_frame{
		u_char DSAP;
		u_char SSAP;
		u_char control_field;
		u_char organisation_code[3];
		u_short type;
	};


	enum IEEE80211_FRAME_TYPE{
		#define MANAGEMENT_FRAME_HDRLEN 24
		MANAGEMENT_FRAME,

		#define CONTROL_FRAME_HDRLEN 16
		CONTROL_FRAME,

		#define LLC_HEADER_LEN		
		DATA_FRAME,
		UNKNOWN_FRAME			
	};
	
	enum IEEE80211_FRAME_SUBTYPE{
		PROBE_REQUEST_FRAME,
		PROBE_RESPONSE_FRAME,
		BEACON_FRAME,
		ACK_REQUEST_FRAME,
		AUTHENTICATION_FRAME,
		ASSOCIATION_REQUEST_FRAME,
		ASSOCIATION_RESPONSE_FRAME,
		
		#define QoSHeaderLen(ieee80211_hdr) ((ieee80211_hdr->frame_control & 0x4000)>0 ? 34 : 26)
		#define IEEE_8021X_AUTHENTICATION 0x8e88
		QOS_DATA_FRAME,
		ACTION_FRAME,
		UNKNOWN_SUBFRAME
	};
	
	
	#define TAGGED_SSID 0
	#define TAGGED_SUPPORTED_RATE 1
	#define TAGGED_DS_PARAM_SET 3
	#define TAGGED_COUNTRY_INFO 7
	#define TAGGED_POWER_CONSTRAINT 32
	#define TAGGED_RSN_INFO 48
	#define TAGGED_HT_CAP 45
	#define TAGGED_HT_INFO 61
	#define TAGGED_EXTENDED_CAP 127
	#define TAGGED_VHT_CAP 191
	#define TAGGED_VHT_OPERATION 192
	#define TAGGED_VENDOR 221
	
	enum IEEE80211_FRAME_TYPE get80211FrameType(const struct ieee80211_frame* frame);
	enum IEEE80211_FRAME_SUBTYPE get80211FrameSubType(const struct ieee80211_frame* frame, enum IEEE80211_FRAME_TYPE type);
	
	int getTaggedValue(const struct ieee80211_management_frame* frame, int tagged_parameter, int packetlen, void* data);
	
#endif




