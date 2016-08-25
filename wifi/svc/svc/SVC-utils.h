#ifndef	__SVC_UTILS__
#define __SVC_UTILS__

	#include "SVC-header.h"
	#include "shared_mutex.h"
	
	#include <cstring>
	#include <vector>
	#include <sys/time.h>
	#include <sys/socket.h>

	typedef void (*SVCDataReceiveHandler)(const uint8_t* data, size_t datalen, void* args);
	
	struct SVCDataReceiveNotificator{
		SVCDataReceiveHandler handler;
		void* args;
		pthread_t thread;			
	};

	class Message{
		public:
			uint8_t* data;
			size_t len;
			
			Message(const uint8_t* data, size_t len){
				this->data = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
				this->len = len;
				memcpy(this->data, data, this->len);
			}
			
			~Message(){
				delete data;
				//printf("message destructed\n");
			}		
	};

	/*	CLASSES DEFINITIONS	*/
	class SVCCommandParam{
	
		bool copy;
		public:
			uint16_t len;
			uint8_t* data;
						
			SVCCommandParam(){
				this->copy = false;
			}
					
			SVCCommandParam(uint16_t length, const uint8_t* data){
				this->len = length;
				this->data = (uint8_t*)malloc(len);
				memcpy(this->data, data, len);
				this->copy = true;
			}
		
			~SVCCommandParam(){	
				if (this->copy){
					delete this->data;
					//printf("param destructed\n");
				}
			}
	};

	/*	just make sure that there will be no wait for 2 same cmd on a single list	*/	
	class SignalNotificator{
		private:			
			struct SVCDataReceiveNotificator* notificationArray[_SVC_CMD_COUNT];
			shared_mutex notificationArrayMutex;
			
			static void waitCommandHandler(const uint8_t* buffer, size_t datalen, void* args);
			
		public:
			SignalNotificator(){
				/*	need to init this array to NULL, otherwise left memory will cause addNotificator to throw exception	*/
				for (uint8_t cmd = 0; cmd<_SVC_CMD_COUNT; cmd++){
					this->notificationArray[cmd] = NULL;
				}			
			}
			~SignalNotificator(){}
			
			SVCDataReceiveNotificator* getNotificator(enum SVCCommand cmd);			
			void removeNotificator(enum SVCCommand cmd);
			void addNotificator(enum SVCCommand cmd, SVCDataReceiveNotificator* notificator);			
			bool waitCommand(enum SVCCommand cmd, vector<SVCCommandParam*>* params, int timeout);
	};
			
	/*	END OF CLASSES DEFINITIONS	*/	

	//--	UTILS FUNCTIONS		--//
	
	//--	return if the command must be encrypted
	bool isEncryptedCommand(enum SVCCommand command);

	//--	clear all params in the vector and call their destructors
	void clearParams(vector<SVCCommandParam*>* params);

	//--	extract parameters from a buffer without header
	void extractParams(const uint8_t* buffer, vector<SVCCommandParam*>* params);

	//--	print current buffer in hex bytes
	void printBuffer(const uint8_t* buffer, size_t len);

	//--	timeoutSignal and waitingSignal must be differrent, otherwise the behavior is undefined
	bool waitSignal(int waitingSignal, int timeoutSignal, int timeout);
	bool waitSignal(int waitingSignal);
	
#endif
