#include "usermain.h"
#include "kernel.h"
#include "schedule.h"
#include <dos.h>
#include "system.h"

PCB **pcbs=new PCB*[50];
int pcnt=0;
KernelSem **sems=new KernelSem*[50];
int scnt=0;
KernelEv **kevs=new KernelEv*[50];
int kcnt=0;

PCB* PCB::running=0;
IVTEntry** IVTEntry::table=new IVTEntry*[256];

SleepQueue* PCB::sq=new SleepQueue();
extern Parameters *par;


/*---------------------PCB START---------------------------*/

PCB::PCB(StackSize stackSize, Time timeSlice, Thread* myThread){ 
	this->myThread=myThread;
	this->stackSize=stackSize;
	timeToLive=timeSlice;
	timeToSleep=0;
	blockedOnMe=0;
	status=AVAILABLE;
	next=0;
}

PCB::PCB(int a){ 
	switch(a){
		case 0:{//main
			stackSize=4096;
			timeToLive=0;
			status=AVAILABLE;
			break;
		}
		case 1:{//idle
			stackSize=512;
			timeToLive=1;
			status=IDLE;
			startIdle();
			break;
		}
		case 2:{//kernel
			stackSize=1024;
			timeToLive=0;
			status=BLOCKED;		
			stack = new unsigned[stackSize];
			break;
		}
	}
	myThread=0;
	timeToSleep=0;
	blockedOnMe=0;
	next=0;
}

void PCB::start(){
	stack = new unsigned[stackSize];
	unsigned temp=stackSize;
	stack[--temp]=FP_SEG(myThread);	
	stack[--temp]=FP_OFF(myThread);
	--temp;
	--temp;
	stack[--temp]=0x200;
	stack[--temp]=FP_SEG(&(Thread::wrapper)); 
	stack[--temp]=FP_OFF(&(Thread::wrapper));	
	temp=stackSize-16;
	ss=FP_SEG(stack+temp);			
	sp=FP_OFF(stack+temp);			
	bp=FP_OFF(stack+temp);
	Scheduler::put(this);
}

void PCB::startKernel(){	
	unsigned temp=stackSize;
	stack[--temp]=FP_SEG(par);	
	stack[--temp]=FP_OFF(par);
	stack[--temp]=FP_SEG(system_call_end);
	stack[--temp]=FP_OFF(system_call_end);
	stack[--temp]=0x200;
	stack[--temp]=FP_SEG(kernel); 
	stack[--temp]=FP_OFF(kernel);	
	ss=FP_SEG(stack+stackSize-16);			
	sp=FP_OFF(stack+stackSize-16);			
	bp=FP_OFF(stack+stackSize-16);
}
void PCB::startIdle(){
	stack = new unsigned[stackSize];
	unsigned temp=stackSize-4;
	stack[--temp]=0x200;
	stack[--temp]=FP_SEG(&(Idle::run)); 
	stack[--temp]=FP_OFF(&(Idle::run));	
	temp=stackSize-16;
	ss=FP_SEG(stack+temp);			
	sp=FP_OFF(stack+temp);			
	bp=FP_OFF(stack+temp);
}

void PCB::finalize(){
	if(running->blockedOnMe!=0){
		running->blockedOnMe->status=AVAILABLE;
		Scheduler::put(running->blockedOnMe);
		running->blockedOnMe=0;
	}
	running->status=FINISHED;	
	dispatch();
}


void PCB::sleep(Time t){
	if(t>0){
		running->status = SLEEPING;
		sq->insert(running,t);
		dispatch();
	}
}

void PCB::waitToComplete(){
	if(status!=FINISHED){
		running->status=BLOCKED;
		blockedOnMe=running;
		dispatch();	
	}
}
PCB::~PCB(){
	next=0;
	delete stack;
}
/*---------------------PCB END-----------------------------*/




/*---------------------SleepQueue START--------------------*/
void SleepQueue::insert(PCB* p, Time time){
	p->status=SLEEPING;
	cnt++;
	if(head==0) head=new Elem(p,time);
	else {
		Elem *prev=0, *temp=head;
		while(temp!=0){
			if(temp->pcb->timeToSleep<=time) time-=temp->pcb->timeToSleep;
			else{
				temp->pcb->timeToSleep-=time;
				break;
			}
			prev=temp;
			temp=temp->next;
		}
		Elem *help=new Elem(p,time);
		if(prev==0){
			help->next=head;
			head=help;
		}
		else{
			help->next=temp;
			prev->next=help;
		}
	}
}

void SleepQueue::decrement(){
	if(head!=0){			
		head->pcb->timeToSleep--;			
		while((head->pcb->timeToSleep)==0){
				Elem* temp = head;
				head->pcb->status = AVAILABLE;					
				head = head->next;
				Scheduler::put(temp->pcb);
				temp->next=0;
				--cnt;	
				delete temp;
				if(head == 0)break;			
		}
	}
}

/*---------------------SleepQueue END----------------------*/


/*---------------------Idle START--------------------------*/
PCB *Idle::pcb=0;

Idle::Idle(){
	pcb=new PCB(1);
}

void Idle::run(){
	while(!UserMain::endOfMain);
}

/*---------------------Idle END----------------------------*/




/*---------------------KernelSem START----------------------*/
KernelSem::KernelSem(int init){ 
	value=init;
	head=0;
	last=0;
} 
void KernelSem::wait(){
	if(--value<0){
		if(head==0) head=PCB::running;
		else last->next=PCB::running;
		last=PCB::running;
		last->status=BLOCKED;
		dispatch();
	}
}
void KernelSem::signal(){ 
	if(value++<0){
		head->status=AVAILABLE;
		Scheduler::put(head);
		PCB* temp=head;
		head=head->next;
		temp->next=0;	
	}
}
void KernelSem::signalAll(){
	while(head!=0){
		head->status=AVAILABLE;
		Scheduler::put(head);
		PCB* temp=head;
		head=head->next;
		temp->next=0;
	}
}
int KernelSem::val(){ return value; }

KernelSem::~KernelSem(){ signalAll();}

/*---------------------KernelSem END------------------------*/


/*---------------------KernelEv START-----------------------*/
KernelEv::KernelEv(IVTNo ivtNo,PCB* myPCB){
	this->ivtNo=ivtNo;
	this->myPCB=myPCB;
	IVTEntry::table[ivtNo]->kev=this;
}

void KernelEv::wait(){
	if(PCB::running==myPCB){
		asm{
			pushf 
			cli
		}
		if(ivtNo == 0x09){
			asm{  		
				mov al,20h
				out 20h,al
			}
		}
		myPCB->status=BLOCKED;
		asm popf;
	}
	dispatch();
}
void KernelEv::signal(){
	if(myPCB->status == BLOCKED){
		myPCB->status=AVAILABLE;
		Scheduler::put(myPCB);
	}
	dispatch();
}

KernelEv::~KernelEv(){
	IVTEntry::table[ivtNo]->kev=0;
}

/*---------------------KernelEv END------------------------*/



/*---------------------IVTEntry START----------------------*/

IVTEntry::IVTEntry(IVTNo no, void interrupt (*newRout)(...)){
	table[no]=this;
	this->no=no;
	oldRout=getvect(no);
	setvect(no,newRout);
}

void IVTEntry::signal(){ kev->signal(); }

void IVTEntry::callOldRout(){ (*oldRout)(); }

IVTEntry::~IVTEntry(){
	callOldRout();
	setvect(no,oldRout);
	table[no]=0;
}

/*---------------------IVTEntry END-------------------------*/



/*---------------------KERNEL FUNCTION----------------------*/
void kernel(Parameters *par){
	switch(par->caller){
		case 1:{
			switch(par->function){
				case 1:{
					if((pcnt%50==0)&&(pcnt!=0)){
						PCB **help=new PCB*[pcnt+50];
						for(int i=0; i<pcnt; i++) help[i]=pcbs[i];
						delete [] pcbs;
						pcbs=help;
					}
					pcbs[pcnt]=new PCB(par->ulParam[0],par->uiParam[0],(Thread*)par->ptr[0]);
					par->returnID=pcnt++;
					break;
				}
				case 2:{
					pcbs[par->reqID]->start();					
					break;
				}
				case 3:{
					PCB::sleep(par->uiParam[0]);
					break;
				}
				case 4:{
					PCB::finalize();
					break;
				}
				case 5:{					
					pcbs[par->reqID]->waitToComplete();
					break;
				}
				case 6:{
					delete pcbs[par->reqID];
					pcbs[par->reqID]=0;
					break;
				}
			}
			break;
		}
		case 2:{			
			switch(par->function){
				case 1:{
					if((scnt%50==0)&&(scnt!=0)){
						KernelSem **help=new KernelSem*[scnt+50];
						for(int i=0; i<scnt; i++) help[i]=sems[i];
						delete [] sems;
						sems=help;
					}
					sems[scnt]=new KernelSem(par->iParam[0]);
					par->returnID=scnt++;
					break;
				}
				case 2:{
					sems[par->reqID]->wait();
					break;
				}
				case 3:{
					sems[par->reqID]->signal();
					break;
				}
				case 4:{
					par->iParam[2]=sems[par->reqID]->val();
					break;
				}
				case 5:{
					delete sems[par->reqID];
					sems[par->reqID]=0;
					break;
				}
			}
			break;
		}
		case 3:{
			switch(par->function){
				case 1:{
					if((kcnt%50==0)&&(kcnt!=0)){
						KernelEv **help=new KernelEv*[kcnt+50];
						for(int i=0; i<kcnt; i++) help[i]=kevs[i];
						delete [] kevs;
						kevs=help;
					}
					kevs[kcnt]=new KernelEv(par->cParam,PCB::running);
					par->returnID=kcnt++;
					break;
				}
				case 2:{					
					kevs[par->reqID]->wait();
					break;
				}
				case 3:{					
					kevs[par->reqID]->signal();
					break;
				}
				case 4:{
					delete kevs[par->reqID];
					kevs[par->reqID]=0;
					break;
				}
			}
			break;
		}
	}
}




