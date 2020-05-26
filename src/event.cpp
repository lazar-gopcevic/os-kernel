#include "event.h"
#include <dos.h>
#include "param.h"

unsigned int sendSEGE,sendOFFE;

Event::Event(IVTNo ivtNo){
	Parameters param;
	param.caller = 3;
	param.function = 1;
	param.cParam = ivtNo;
	sendSEGE = FP_SEG(&param);
	sendOFFE = FP_OFF(&param);
	asm{
		mov bx,sendSEGE;
		mov cx,sendOFFE;
		int 61h;
	}
	id = param.returnID;
}
void Event::wait(){
	Parameters param;
	param.caller = 3;
	param.function = 2;
	param.reqID = id;
	sendSEGE = FP_SEG(&param);
	sendOFFE = FP_OFF(&param);
	asm{
		mov bx,sendSEGE;
		mov cx,sendOFFE;
		int 61h;
	}
}
void Event::signal(){
	Parameters param;
	param.caller = 3;
	param.function = 3;
	param.reqID = id;
	sendSEGE = FP_SEG(&param);
	sendOFFE = FP_OFF(&param);
	asm{
		mov bx,sendSEGE;
		mov cx,sendOFFE;
		int 61h;
	}
} 

Event::~Event(){
	Parameters param;
	param.caller = 3;
	param.function = 4;
	param.reqID = id;
	sendSEGE = FP_SEG(&param);
	sendOFFE = FP_OFF(&param);
	asm{
		mov bx,sendSEGE;
		mov cx,sendOFFE;
		int 61h;
	}
}