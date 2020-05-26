#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "event.h"
#include "thread.h"
#include "semaphor.h"
#include "param.h"

void kernel(Parameters*);

enum Status {AVAILABLE, FINISHED, BLOCKED, SLEEPING, IDLE, MAIN};

/*-----------------------------------------------------*/
class Idle{
public:
	static PCB *pcb;
	Idle();
	static void run();
};

/*-----------------------------------------------------*/
class SleepQueue;

/*-----------------------------------------------------*/
class PCB{
public:
	
	unsigned ss,sp,bp;
	unsigned *stack;
	StackSize stackSize;
	Time timeToLive, timeToSleep;
	Thread *myThread;
	PCB* blockedOnMe;
	Status status;
	
	static PCB* running;
	static SleepQueue* sq;
	PCB* next;
	
	PCB(StackSize, Time, Thread*);
	PCB(int);
	
	void start();
	void startKernel();
	void startIdle();
	static void finalize();
	static void sleep(Time);
	void waitToComplete();
	~PCB();	
};

/*-----------------------------------------------------*/
struct Elem{
	PCB *pcb;
	Elem *next;
	Elem(PCB* p, Time t){
		next=0;
		pcb=p;
		pcb->timeToSleep=t;
	}
};

/*-----------------------------------------------------*/
class SleepQueue{
public:
	Elem *head;
	int cnt;
	SleepQueue(){
		head=0;
		cnt=0;
	}
	void insert(PCB* p, Time t);
	void decrement();
};

/*-----------------------------------------------------*/
class KernelSem{
	int value;
	PCB* head;
	PCB* last;
public:
	KernelSem(int);
	
	void wait();
	void signal();
	void signalAll();
	int val();
	
	~KernelSem();
};

/*-----------------------------------------------------*/
class KernelEv{
	PCB* myPCB;
	IVTNo ivtNo;
public:
	KernelEv(IVTNo,PCB*);	
	void wait();
	void signal();	
	~KernelEv();
};
/*-----------------------------------------------------*/
class IVTEntry{
public:
	KernelEv *kev;
	void interrupt (*oldRout)(...);
	IVTNo no;
	static IVTEntry** table;
	
	IVTEntry(IVTNo,void interrupt (*vect)(...));
	IVTEntry *getEntry(IVTNo);
	void signal();
	void callOldRout();
	~IVTEntry();
};




#endif