#include "semaphor.h"
#include "param.h"
#include <dos.h>

unsigned int sendSEGS,sendOFFS;

Semaphore::Semaphore(int init){
	Parameters param;
	param.caller = 2;
	param.function = 1;
	param.iParam[0] = init;
	sendSEGS = FP_SEG(&param);
	sendOFFS = FP_OFF(&param);
	asm{
		mov bx,sendSEGS
		mov cx,sendOFFS
		int 61h
	}
	id = param.returnID;
}

void Semaphore::wait(){
	Parameters param;
	param.caller = 2;
	param.function = 2;
	param.reqID = id;
	sendSEGS = FP_SEG(&param);
	sendOFFS = FP_OFF(&param);
	asm{
		mov bx,sendSEGS;
		mov cx,sendOFFS;
		int 61h;
	}
}

void Semaphore::signal(){
	Parameters param;
	param.caller = 2;
	param.function = 3;
	param.reqID = id;
	sendSEGS = FP_SEG(&param);
	sendOFFS = FP_OFF(&param);
	asm{
		mov bx,sendSEGS;
		mov cx,sendOFFS;
		int 61h;
	}
}

int Semaphore::val() const{
	Parameters param;
	param.caller = 2;
	param.function = 4;
	param.reqID = id;
	sendSEGS = FP_SEG(&param);
	sendOFFS = FP_OFF(&param);
	asm{
		mov bx,sendSEGS;
		mov cx,sendOFFS;
		int 61h;
	}
	return param.iParam[2];
}

Semaphore::~Semaphore(){
	Parameters param;
	param.caller = 2;
	param.function = 5;
	param.reqID = id;
	sendSEGS = FP_SEG(&param);
	sendOFFS = FP_OFF(&param);
	asm{
		mov bx,sendSEGS;
		mov cx,sendOFFS;
		int 61h;
	}
}
