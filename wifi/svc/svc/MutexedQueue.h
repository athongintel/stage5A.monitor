#ifndef __SVC_QUEUE__
#define __SVC_QUEUE__

	#include "Node.h"
	#include "shared_mutex.h"
	#include "SVC-utils.h"
	
	#define QUEUE_DATA_SIGNAL	SIGUSR2

	using namespace std;

	/* 
		Author: Immort
		This queue is implemented to be generic and thread-safe
	*/

	template <class T>
	class MutexedQueue{
	
		private:
			Node<T>* first;
			Node<T>* last;			
			int count;
			
			shared_mutex* countMutex;
			shared_mutex* firstMutex;
			shared_mutex* lastMutex;
			
			Queue<pthread_t>* waitDataThreads;
						
			//--	waitData used in mutex lock, not need to lock again
			bool waitData(){
				waitDataThreads->enqueue(pthread_self());
				return waitSignal(QUEUE_DATA_SIGNAL);
			}
			
			//--	signalThread used in mutex lock, not need to lock again
			void signalThread(){
				pthread_t thread;
				if (waitDataThreads->peak(&thread)){
					waitDataThreads->dequeue();
					pthread_kill(thread, QUEUE_DATA_SIGNAL);
				}
			}
			
		public:
	
			MutexedQueue(){
				this->first = NULL;
				this->last = NULL;
				this->count = 0;
				countMutex = new shared_mutex();
				firstMutex = new shared_mutex();
				lastMutex = new shared_mutex();
				waitDataThreads = new Queue<pthread_t>();								
			}
		
			~MutexedQueue(){
				while (this->notEmpty()){
					delete this->dequeue();
				}
				delete this->waitDataThreads;
			}
		
			bool notEmpty(){			
				bool rs;
				this->countMutex->lock_shared();
				rs = count>0;
				this->countMutex->unlock_shared();
				return rs;
			}

			void enqueue(T data){
				Node<T>* element = new Node<T>();
				element->setData(data);
				element->setNext(NULL);
				
				this->firstMutex->lock();
				if (this->notEmpty()){
					this->lastMutex->lock();
					this->last->setNext(element);
					this->last = element;
					this->lastMutex->unlock();
				}
				else{				
					this->lastMutex->lock();
					this->first = element;
					this->last = element;
					this->lastMutex->unlock();
					signalThread();
				}
				this->countMutex->lock();
				this->count++;
				this->countMutex->unlock();
				this->firstMutex->unlock();
			}
			
			T dequeueWait(){
				this->firstMutex->lock();
				bool haveData = true;
				if (!this->notEmpty()){
					haveData = waitData();
					//--	after waitData there must be data in queue, 'cause no other can perform dequeue
				}
				//--	not empty, have not to wait
				if (haveData){
					Node<T>* tmp = this->first;
					this->first = tmp->getNext();																				
					this->countMutex->lock();
					this->count--;				
					this->countMutex->unlock();				
					this->firstMutex->unlock();
					return tmp->getData();
				}
				//--	else: waitData interrupted by other signals
				return NULL;
			}
			
			T dequeue(){
				this->firstMutex->lock();
				if (this->notEmpty()){					
					Node<T>* tmp = this->first;
					this->first = tmp->getNext();					
					
					this->countMutex->lock();
					this->count--;				
					this->countMutex->unlock();
					
					this->firstMutex->unlock();
					return tmp->getData();										
				}
				this->firstMutex->unlock();
				return NULL;
			}
			
			bool peak(T* data){
				if (this->notEmpty()){
					this->firstMutex->lock_shared();
					*data = this->first->getData();
					this->firstMutex->unlock_shared();
					return true;
				}
				return false;
			}
	};

#endif
	
