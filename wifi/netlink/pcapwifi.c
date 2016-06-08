#include "pcapwifi.h"


enum IEEE80211_FRAME_TYPE get80211FrameType(const struct ieee80211_frame* frame){
	u_char frame_type = (frame->frame_control & 0x0C)>>2;

	switch (frame_type){
		case 0: 
			return MANAGEMENT_FRAME;
		case 1:
			return CONTROL_FRAME;
		case 2:
			return DATA_FRAME;
		default:
			return UNKNOWN_FRAME;
	}
}

enum IEEE80211_FRAME_SUBTYPE get80211FrameSubType(const struct ieee80211_frame* frame, enum IEEE80211_FRAME_TYPE type){
	u_char frame_control = frame->frame_control;
	u_char frame_subtype = (frame_control & 0xF0)>>4;
	switch (type){
		case MANAGEMENT_FRAME: 
			switch (frame_subtype){
				case 0:
					return ASSOCIATION_REQUEST_FRAME;
				case 1:
					return ASSOCIATION_RESPONSE_FRAME;
				case 5:
					return PROBE_RESPONSE_FRAME;
				case 8:
					return BEACON_FRAME;
				case 11:
					return AUTHENTICATION_FRAME;
				case 13:
					return ACTION_FRAME;
				default:
					return UNKNOWN_SUBFRAME;		
			}
		case CONTROL_FRAME:
			switch (frame_subtype){
				case 8:
					return ACK_REQUEST_FRAME;
				default:
					return UNKNOWN_SUBFRAME;		
			}
		case DATA_FRAME:
			switch (frame_subtype){	
				case 8:
					return QOS_DATA_FRAME;		
				default:
					return UNKNOWN_SUBFRAME;		
			}
		default:
			return UNKNOWN_SUBFRAME;
	}
}

u_char* getTaggedValue(const struct ieee80211_management_frame* frame, int tagged_parameter, int len){
	
	u_char* result = NULL;
	u_char* tags = &(frame->taggedParameters);
	if (tags == NULL){
		printf("tag = NULL!!\n");
	}
	else{
		int searchLen = 0;
		int tagLen = 0;
		do{			
			if ((*tags) == tagged_parameter){
				//pointer at tag is tag ID, at tag+1 is tag len, tag+2 is the starting value
				tagLen = *(tags+1);
				result = (u_char*)malloc(tagLen);
				memcpy(result, tags+2, tagLen);
				return result;
			}
			else{
				searchLen += tagLen + 2;	
			}
		}
		while (searchLen<len);
	}
	return result;
}
