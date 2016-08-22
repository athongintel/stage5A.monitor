#ifndef __SVC_QUEUE__
#define __SVC_QUEUE__

	#include "Node.h"
	#include "shared_mutex"

	using namespace std;

	/* 
		Author: Immort
		This queue is implemented to be generic and thread-safe
	*/

	template <class T>
	class Queue{
	
		private:
			Node<T>* first;
			Node<T>* last;			
			int count;
			
			shared_mutex* countMutex;
			shared_mutex* firstMutex;
			shared_mutex* lastMutex;

		public:
	
			Queue(){
				this->first = NULL;
				this->last = NULL;
				this->count = 0;
				countMutex = new shared_mutex();
				firstMutex = new shared_mutex();
				lastMutex = new shared_mutex();
			}
		
			~Queue(){
				while (this->notEmpty()){
					this->dequeue();
				}
			}
		
			bool notEmpty(){			
				bool rs;
				this->countMutex.lock_shared();
				rs = count>0;
				this->countMutex.unlock_shared();
				return rs;
			}

			void enqueue(T data){
				Node<T>* element = new Node<T>();
				element->setData(data);
				element->setNext(NULL);
				
				if (this->notEmpty()){
					this->lastMutex->lock();
					this->last->setNext(element);
					this->last = element;
					this->lastMutex->unlock();
				}
				else{
					this->firstMutex->lock();
					this->lastMutex->lock();
					this->first = element;
					this->last = element;
					this->lastMutex->unlock();
					this->firstMutex->unlock();				
				}
				this->countMutex->lock();
				this->count++;
				this->countMutex->unlock();
			}
			
			void dequeue(){
				if (this->notEmpty()){
					this->firstMutex->lock();
					Node<T>* tmp = this->first;
					this->first = tmp->getNext();
					delete tmp;
					this->firstMutex->unlock();
				}
				/*
				else: queue is empty, do nothing
				*/
				this->countMutex.lock();
				this->count--;				
				this->countMutex.unlock();
			}
		
			bool peak(T* data){
				bool rs;				
				if (this->notEmpty()){
					this->firstMutex->lock_shared();
					*data = this->first->getData();
					this->firstMutex->unlock_shared();
					rs = true;
				}
				else{
					rs = false;
				}				
				return rs;				
			}						
	};

#endif
	
