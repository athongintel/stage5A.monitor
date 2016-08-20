/*		SVC-header contents common functionalities used by both SVC and SVC-daemon*/

#ifndef __SVC_HEADEAR__
#define __SVC_HEADER__

	#include "Queue.h"

	#include <vector>
	#include <string>
	#include <cstring>
	#include <mutex>

	#include <shared_mutex>
	#include <csignal>
	#include <vector>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <functional>
	#include <unistd.h>
	
	/*	COMPTIBILITY	*/
	#define sigev_notify_thread_id _sigev_un._tid
	
	
	using namespace std;

	/*	HTP definitions */
	#define AF_HTP								AF_PHONET
	#define PF_HTP								PF_PHONET
	
	

	/*	SVC ERROR DESCRIPTION	*/
	#define SVC_ERROR_NAME_EXISTED				"Application is already running"
	#define SVC_ERROR_UNEXPECTED_RESPONSE		"Unexpected response"
	#define SVC_ERROR_NOT_ESTABLISHED			"Connection not established"
	#define SVC_ERROR_REQUEST_TIMEDOUT			"Request timed out"
	#define SVC_ERROR_AUTHENTICATION_FAILED		"Authentication failed"
	#define SVC_ERROR_CRITICAL					"Critical error"
	#define SVC_ERROR_BINDING					"Error binding socket"
	#define SVC_ERROR_NOTIFICATOR_DUPLICATED	"Notificator duplicated"	

	/*	SVC CONSTANTS	*/
	#define SVC_ACQUIRED_SIGNAL					SIGUSR1
	#define SVC_TIMEOUT_SIGNAL					SIGUSR2
	#define SVC_SHARED_MUTEX_SIGNAL				SVC_ACQUIRED_SIGNAL

	#define SVC_DEFAULT_TIMEOUT 				2000
	#define SVC_SHORT_TIMEOUT					100
	#define SVC_DEFAULT_BUFSIZ 					65536

	/*	SVC INFO BIT	*/
	#define SVC_COMMAND_FRAME  					0x80
	#define SVC_DATA_FRAME  					0x00
	#define SVC_ENCRYPTED						0x08
	#define SVC_USING_TCP						0x04
	
	#define SVC_URGENT_PRIORITY 				0x03
	#define	SVC_HIGH_PRIORITY					0x02
	#define SVC_NORMAL_PRIORITY					0x01
	#define SVC_LOW_PRIORITY					0x00

	static uint32_t SVC_DEFAULT_SESSIONID = 	0x00000000;
	static string SVC_DAEMON_PATH = 			"/tmp/svc-daemon";
	static string SVC_CLIENT_PATH_PREFIX = 		"/tmp/svc-client-";
	
	
	/*	ABI, DO NOT MODIFY UNLESS YOU KNOW EXACTLY WHAT	YOU DO	*/
	enum SVCCommand : uint8_t{
		SVC_CMD_CHECK_ALIVE,
		SVC_CMD_REGISTER_APP,
		SVC_CMD_CONNECT_STEP1,
		SVC_CMD_CONNECT_STEP2,
		SVC_CMD_CONNECT_STEP3,
		SVC_CMD_CONNECT_STEP4,
		_SVC_CMD_COUNT
	};
	/*	END OF ABI	*/

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
			}		
	};


	/*	CLASSES DEFINITIONS	*/
	class SVCCommandParam{
	
		bool copy;
		public:
			uint16_t length;
			uint8_t* param;
			
			
			SVCCommandParam(){
				this->copy = false;
			}
					
			SVCCommandParam(uint16_t length, const uint8_t* param){
				this->length = length;
				this->param = (uint8_t*)malloc(length);
				memcpy(this->param, param, length);
				this->copy = true;
			}
		
			~SVCCommandParam(){	
				printf("param destructed\n");		
				if (this->copy) delete param;
			}
	};
	
	class shared_mutex{	
		
		private:			
			int readerPresence;	
			int writerPresence;
				
			Queue<pthread_t> readWaitQueue;
			Queue<pthread_t> writeWaitQueue;
		
			mutex readerPresenceMutex;
			mutex writerPresenceMutex;
			mutex readWaitQueueMutex;
			mutex writeWaitQueueMutex;
	
			void waitSignal(){
				sigset_t sigset;
				sigemptyset(&sigset);
				sigaddset(&sigset, SVC_SHARED_MUTEX_SIGNAL);
				/*	no need to check for caughtSignal because we only block only one signal	*/
				int caughtSignal;
				/*	TODO: check return value of sigwait	*/
				sigwait(&sigset, &caughtSignal);
				return;
			}
	
		public:
	
			shared_mutex(){
				readerPresence = 0;
				writerPresence = 0;
			}
		
			void lock(){
				/*	set writer presence if not set yet	*/
				writerPresenceMutex.lock();
				//printf("lock with writerPresence %d\n", writerPresence);
				if (writerPresence == 0){
					/* there is no writer, we are the only one	*/
					writerPresenceMutex.unlock();
					/*	check for reader(s)	*/
					readerPresenceMutex.lock();
					if (readerPresence==0){
						/*	there is no reader, we can lock	*/
						readerPresenceMutex.unlock();
						writerPresenceMutex.lock();
						writerPresence++;
						writerPresenceMutex.unlock();
						return;
					}
					else{
						readerPresenceMutex.unlock();
						/*	set writerPresence so no more reader can lock_shared	*/
						writerPresenceMutex.lock();
						writerPresence++;
						writerPresenceMutex.unlock();
						/*	wait for the last reader to notify	*/
						writeWaitQueueMutex.lock();
						writeWaitQueue.enqueue(pthread_self());
						writeWaitQueueMutex.unlock();
						waitSignal();
						/*	no more reader, we lock	*/
						return;
					}				
				}
				else{
					/*
						there are other writers, wait in queue to be notified	
						this also means there is absolutely no reader
					*/
					writerPresence++;
					writerPresenceMutex.unlock();
					/*	add this thread to queue	*/
					writeWaitQueueMutex.lock();
					writeWaitQueue.enqueue(pthread_self());
					writeWaitQueueMutex.unlock();
					/*	wait for signal from preceeded thread	*/
					waitSignal();
					/*	we've just been signaled from other thread, now we lock	*/				
					return;
				}
			}
		
			void lock_shared(){
				/* check if there is any pending writer	*/
				writerPresenceMutex.lock();
				//printf("lockshare with writerPresence %d\n", writerPresence);
				if (writerPresence == 0){
					/*	no writer, we can lock_shared	*/
					writerPresenceMutex.unlock();
					readerPresenceMutex.lock();
					readerPresence++;
					readerPresenceMutex.unlock();
					return;
				}
				else{
					/*	
						there is at least one writer has obtained the mutex or JUST WANTED TO enter the critical session
						we can no more allow reader to enter the queue, but after the last writer releases the mutex						
					*/					
					writerPresenceMutex.unlock();
					
					/*	wait here for the last writer to notify	*/					
					readWaitQueueMutex.lock();
					readWaitQueue.enqueue(pthread_self());
					readWaitQueueMutex.unlock();
					waitSignal();
					
					/*	notified, we can lock_shared	*/
					readerPresenceMutex.lock();
					readerPresence++;
					readerPresenceMutex.unlock();
					
					return;
				}
			}
		
			void unlock_shared(){
				readerPresenceMutex.lock();
				//printf("unlock shared with readerPresence %d\n", readerPresence);
				if (readerPresence == 0){
					/*	there is no lock_shared to be unlock_shared	*/			
					readerPresenceMutex.unlock();
					return;
				}
				else{
					readerPresence--;
					if (readerPresence == 0){
						readerPresenceMutex.unlock();
						/*	we are the last reader, notify the first writer (if any) to lock	*/
						writeWaitQueueMutex.lock();
						if (writeWaitQueue.notEmpty()){
							/*	this must be the first writer waiting	*/
							pthread_t* tid;
							writeWaitQueue.peak(tid);
							pthread_kill(*tid, SVC_SHARED_MUTEX_SIGNAL);
							writeWaitQueue.dequeue();
							/*	job done, return	*/
							writeWaitQueueMutex.unlock();
							return;
						}
						else{
							writeWaitQueueMutex.unlock();
							return;
						}
					}
					else{
						readerPresenceMutex.unlock();
						return;
					}
				}
			}
		
			void unlock(){
				/*	check if there are other writer waiting	*/
				writerPresenceMutex.lock();
				//printf("unlock with writerPresence %d\n", writerPresence);
				if (writerPresence == 0){
					/*	no more lock to unlock	*/
					writerPresenceMutex.unlock();
					return;
				}
				else{
					writerPresence--;
					writerPresenceMutex.unlock();
					/*	notify the next writer if any	*/
					writeWaitQueueMutex.lock();
					if (writeWaitQueue.notEmpty()){
						/*	notify	*/
						pthread_t* tid;
						writeWaitQueue.peak(tid);
						pthread_kill(*tid, SVC_SHARED_MUTEX_SIGNAL);
						writeWaitQueue.dequeue();
						writeWaitQueueMutex.unlock();
						return;
					}
					else{			
						writeWaitQueueMutex.unlock();		
						/*	no more writer, notify all readers	*/
					
						readWaitQueueMutex.lock();
						while (readWaitQueue.notEmpty()){
							pthread_t* tid;
							readWaitQueue.peak(tid);
							pthread_kill(*tid, SVC_SHARED_MUTEX_SIGNAL);
							readWaitQueue.dequeue();
						}
						readWaitQueueMutex.unlock();
						return;
					}
				}
			}
	};


	/*	just make sure that there will be no wait for 2 same cmd on a single list	*/	
	class SignalNotificator{
		static void waitCommandHandler(const uint8_t* buffer, size_t datalen, void* args);		
		public:
			struct SVCDataReceiveNotificator* notificationArray[_SVC_CMD_COUNT];
			shared_mutex notificationArrayMutex;
			
			SignalNotificator(){
				/*	need to init this array to NULL, otherwise left memory will cause addNotificator to throw exception	*/
				for (uint8_t cmd = 0; cmd<_SVC_CMD_COUNT; cmd++){
					this->notificationArray[cmd] = NULL;
				}			
			}
			~SignalNotificator(){}
			
			SVCDataReceiveNotificator* getNotificator(enum SVCCommand cmd){
				SVCDataReceiveNotificator* rs;
				notificationArrayMutex.lock_shared();
				rs = notificationArray[cmd];
				notificationArrayMutex.unlock_shared();
				return rs;
			}
			
			void removeNotificator(enum SVCCommand cmd){
				notificationArrayMutex.lock();
				if (notificationArray[cmd]!=NULL){
					delete notificationArray[cmd];
					notificationArray[cmd]=NULL;
					printf("noti removed, cmd: %d\n", cmd);
				}
				notificationArrayMutex.unlock();				
			}
			
			void addNotificator(enum SVCCommand cmd, SVCDataReceiveNotificator* notificator){
				notificationArrayMutex.lock();
				if (notificationArray[cmd]!=NULL){
					notificationArrayMutex.unlock();
					throw SVC_ERROR_NOTIFICATOR_DUPLICATED;
				}
				else{
					notificationArray[cmd] = notificator;
					notificationArrayMutex.unlock();
					printf("noti added, cmd: %d\n", cmd);
				}					
			}
			bool waitCommand(enum SVCCommand cmd, vector<SVCCommandParam*>* params, int timeout);
	};
			
	/*	END OF CLASSES DEFINITIONS	*/
	

	bool isEncryptedCommand(enum SVCCommand command);	
	void prepareCommand(uint8_t* buffer, size_t* len, uint32_t sessionID, enum SVCCommand command, const vector<SVCCommandParam*>* params);
	void sendCommand(int socket, uint32_t sessionID, enum SVCCommand command, const vector<SVCCommandParam*>* params);	
	ssize_t _sendPacket(int socket, const uint8_t* buffer, size_t len);			
	
	
	/*	UTILS FUNCTIONS	*/
	/*	print current buffer in hex bytes	*/
	void printBuffer(const uint8_t* buffer, size_t len);
	/*	timeoutSignal and waitingSignal must be differrent, or the behavior is undefined*/
	bool waitSignal(int waitingSignal, int timeoutSignal, int timeout);	
	
#endif
