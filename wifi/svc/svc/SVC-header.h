/*		SVC-header contents common functionalities used by both SVC and SVC-daemon*/

#ifndef __SVC_HEADEAR__
#define __SVC_HEADER__

	#include <vector>
	#include <string>
	#include <cstring>
	#include <mutex>
	#include <shared_mutex>
	#include <csignal>
	#include <vector>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <sys/syscall.h>
	#include <functional>
	#include <unistd.h>
	
	/*	COMPTIBILITY	*/
	#define sigev_notify_thread_id _sigev_un._tid
	#define gettid() syscall(SYS_gettid)
	
	
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


	/*	SVC CONSTANTS	*/
	#define SVC_ACQUIRED_SIGNAL					SIGUSR1
	#define SVC_TIMEOUT_SIGNAL					SIGUSR2
	#define SVC_SHARED_MUTEX_SIGNAL				SIGUSR1
	
	#define SVC_DEFAULT_TIMEOUT 				2000
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
	
	
	/*	SVC COMMAND	*/
	enum SVCCommand : uint8_t{
		SVC_CMD_CHECK_ALIVE,
		SVC_CMD_REGISTER_APP,
		SVC_CMD_CONNECT_STEP1,
		SVC_CMD_CONNECT_STEP2,
		SVC_CMD_CONNECT_STEP3,
		SVC_CMD_CONNECT_STEP4
	};
	/*	END OF ABI	*/

	typedef void (*SVCDataReceiveHandler)(const uint8_t* data, size_t datalen, void* args);
	
	struct SVCDataReceiveNotificator{
		SVCDataReceiveHandler handler;
		void* args;		
		enum SVCCommand command;
		pthread_t thread;			
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

	template <class T>
	class Node{
		private:
			T data;
			Node* next;
		public:
			void setData(T data){
				this->data = data;
			}
			void setNext(const Node* next){
				this->next = next;
			}
			T getData(){
				return this->data;
			}
			Node* getNext(){
				return this->next;
			}
	}
	
	class Queue{
	
		private:
			Node<T>* first;
			Node<T>* last;
			int count;			
			mutex countMutex;
	
		/*int head;
		int tail;
		int size;
		int maxsize;
		uint8_t** array;	
		size_t* arrayLen;
		*/

		public:
		
			Queue(){
				this->first = NULL;
				this->last = NULL;
				this->count = 0;
			}
			
			~Queue(){
				while (this->notEmpty()){
					this->dequeue();
				}
			}
			
			bool notEmpty(){
				bool rs;
				this->countMutex.lock();
				rs = count>0;
				this->countMutex.unlock();
				return rs;
			}
			
			void enqueue(T data){
				Node<T>* element = new Node<T>();
				element->setData(T);
				element->setNext(NULL);
				this->countMutex.lock();
				if (this->count==0){
					this->first = element;
					this->last = element;
				}
				else{
					this->last->setNext(element);
					this->last = element;				
				}
				this->count++;
				this->countMutex.unlock();				
			}
			
			bool peak(T* data){
				bool rs;
				this->countMutex.lock();
				if (this->count>0){
					*T = this->first->getData();
					rs = true;
				}
				else{
					rs = false;
				}
				this->countMutex.unlock();
				return rs;				
			}
			
			void dequeue(){
				this->countMutex.lock();
				if (this->count>0){
					
				}
				/*
				else: queue is empty, do nothing
				*/
			}
			
			static const int MESSAGE_QUEUE_DEFAULT_SIZE = 4096;
			mutex messageMutex;
	
			MessageQueue(int maxsize=MessageQueue::MESSAGE_QUEUE_DEFAULT_SIZE);
			~MessageQueue();
		
			bool enqueue(const uint8_t* message, size_t len);
			bool peak(uint8_t** message, size_t* len);
			bool dequeue();
			bool notEmpty();
		
	};
	
	class SignalNotificator{
		static	void waitCommandHandler(const uint8_t* buffer, size_t datalen, void* args);		
		public:
			vector<struct SVCDataReceiveNotificator*> notificationList;
			mutex notificationListMutex;
			
			SignalNotificator(){}
			~SignalNotificator(){}
			
			void removeNotificator(vector<SVCDataReceiveNotificator*>::const_iterator position){
				notificationListMutex.lock();
				notificationList.erase(position);
				printf("noti removed\n");
				notificationListMutex.unlock();
			}
			void addNotificator(SVCDataReceiveNotificator* notificator){
				notificationListMutex.lock();
				notificationList.push_back(notificator);
				printf("noti added\n");
				notificationListMutex.unlock();
			}
			bool waitCommand(enum SVCCommand cmd, vector<SVCCommandParam*>* params, int timeout);
	};
	
	class shared_mutex{	
					
		int readerPresence;
		int writerPresence;		
		vector<pthread_t*> readWaitQueue;
		vector<pthread_t*> writeWaitQueue;
		
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
		}
	
		public:
	
		shared_mutex(){
			readWaitQueue.empty();
			writeWaitQueue.empty();
			readerPrecence = 0;
			writerPresence = 0;
		}
		
		void lock(){
			/*	set writer presence if not set yet*/
			writerPresenceMutex.lock();
			if (writerPresence == 0){
				/* there is no writer, we are the only one	*/
				writerPresenceMutex.unlock();
				/*	check for reader(s)	*/
				readerPresenceMutex.lock();
				if (readerPresence==0){
					/*	there is no reader, we can lock	*/
					readerPresenceMutex.unlock();
					return;
				}
				else{
					readerPresenceMutex.unlock();
					/*	set writerPresence so no more reader can lock or lock_shared	*/
					writerPresenceMutex.lock();
					writerPresence++;
					writerPresenceMutex.unlock();
					/*	wait for the last reader to notify	*/
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
				writeWaitQueue.enqueue(gettid());
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
			if (writerPresence == 0){
				/*	no writer, we can lock_shared	*/
				writerPresenceMutex.unlock();
				return;
			}
			else{
				/*	and to queue to be notified by last writer	*/
				writerPresenceMutex.unlock();
				readerPresenceMutex.lock();
				readerPresence++;
				readerPresenceMutex.unlock();
				readWaitQueueMutex.lock();
				readWaitQueue.enqueue(gettid());
				readWaitQueueMutex.unlock();
				/*	wait to be notified	*/
				waitSignal();
				/*	notified, we can lock_shared	*/
				return;
			}
		}
		
		void unlock_shared(){
			readerPresenceMutex.lock();
			if (readerPresence == 0){
				/*	there is no lock_shared to be unlock_shared	*/
				readerPresenceMutex.unlock();
				return;
			}
			else{
				readerPresence--;
				if (readerPresence == 0){
					/*	we are the last reader, notify writer (if any) to lock	*/
					writeWaitQueueMutex.lock();
					if (writeWaitQueue.notEmpty()){
						/*	this must be the only writer waiting	*/
						pthread_t* tid;
						writeWaitQueue.peak(tid);
						pthread_kill(*tid, SVC_SHARED_MUTEX_SIGNAL);
						writeWaitQueue.dequeue();
						/*	job done, return	*/
						writeWaitQueueMutex.unlock();
					}
				}
				/*
				else: do nothing as we are not the last one
				*/
				readerPresenceMutex.unlock();
				return;
			}
		}
		
		void unlock(){
			/*	check if there are other writer waiting	*/
			writerPresenceMutex.lock();
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
						readerPresenceMutex.lock();
						readerPresence--;
						readerPresenceMutex.unlock();
					}
					readWaitQueueMutex.unlock();
					return;
				}
			}
		}
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
