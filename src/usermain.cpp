#include "usermain.h"
#include "kernel.h"
#include "system.h"
#include <dos.h>

volatile int UserMain::endOfMain=0;
int UserMain::returnValue=-1;
PCB* mainPCB=new PCB(0);

extern int userMain(int argc, char **argv);

void UserMain::execute(int argc, char **argv){
	
	PCB::running=mainPCB;
	Idle *idle= new Idle();
	
	void interrupt (*oldKern)(...)=getvect(0x61);
	void interrupt (*newKern)(...)=&system_call;
	
	setvect(0x61,newKern);
		timerInit();	
			returnValue=userMain(argc,argv);
			endOfMain=1;
		timerRestore();	
	setvect(0x61,oldKern);
	
	delete idle;
}