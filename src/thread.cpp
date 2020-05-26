#include "thread.h"
#include <dos.h>
#include "param.h"

extern volatile int RCS;
unsigned volatile int sendSEG,sendOFF;

Thread::Thread(StackSize stackSize, Time timeSlice){  	
	Parameters param;
	param.caller = 1;
	param.function = 1;
	param.ptr[0] = this;
	param.ulParam[0] = stackSize;
	param.uiParam[0] = timeSlice;
	sendSEG = FP_SEG(&param);
	sendOFF = FP_OFF(&param);	
	asm{
		mov bx,sendSEG;
		mov cx,sendOFF;
		int 61h;
	}
	id = param.returnID;	
}

void Thread::start(){				
	Parameters param;
	param.caller = 1;
	param.function = 2;
	param.reqID = id;
	sendSEG = FP_SEG(&param);
	sendOFF = FP_OFF(&param);
	asm{
		mov bx,sendSEG;
		mov cx,sendOFF;
		int 61h;
	}
}

void Thread::sleep(Time timeToSleep){ 	
	Parameters param;
	param.caller = 1;
	param.function = 3;
	param.uiParam[0] = timeToSleep;
	sendSEG = FP_SEG(&param);
	sendOFF = FP_OFF(&param);
	asm{
		mov bx,sendSEG;
		mov cx,sendOFF;
		int 61h;
	}
}
void Thread::wrapper(Thread *running)
{	
	running->run();
	Parameters param;
	param.caller = 1;
	param.function = 4;
	sendSEG = FP_SEG(&param);
	sendOFF = FP_OFF(&param);
	asm{
		mov bx,sendSEG;
		mov cx,sendOFF;
		int 61h;
	}
}

void Thread::waitToComplete(){							
	Parameters param;
	param.caller = 1;
	param.function = 5;
	param.reqID = id;
	sendSEG = FP_SEG(&param);
	sendOFF = FP_OFF(&param);
	asm{
		mov bx,sendSEG;
		mov cx,sendOFF;
		int 61h;
	}
}

Thread::~Thread(){
	Parameters param;
	param.caller = 1;
	param.function = 6;
	param.reqID = id;
	sendSEG = FP_SEG(&param);
	sendOFF = FP_OFF(&param);
	asm{
		mov bx,sendSEG;
		mov cx,sendOFF;
		int 61h;
	}
}

void dispatch (){ 
	RCS=1;	
	asm int 08h;
} 

