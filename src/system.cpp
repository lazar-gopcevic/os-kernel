#include "system.h"
#include "param.h"
#include "schedule.h"
#include "kernel.h"
#include <dos.h>

PCB* systemPCB = new PCB(2);
Parameters *par;
volatile int system_mode=0;
unsigned parSEG,parOFF;
volatile int timerFlag=0;

extern PCB* mainPCB;
extern void tick();
volatile int RCS=0;
volatile int brojac=0;

unsigned tsp;
unsigned tss;
unsigned tbp;

void interrupt system_call(...){
	system_mode=1;
	asm{
		mov parSEG, bx
		mov parOFF, cx
		
		sti;
		
		mov tsp, sp 
		mov tbp, bp
		mov tss, ss
	}
	

	PCB::running->ss = tss;
	PCB::running->bp = tbp;
	PCB::running->sp = tsp;

	par=(Parameters*)MK_FP(parSEG,parOFF);
	systemPCB->startKernel();

	tss=systemPCB->ss;
	tsp=systemPCB->sp;
	tbp=systemPCB->bp;
	
	asm { 
		mov sp, tsp 
		mov bp, tbp
		mov ss, tss
	}
}

void interrupt system_call_end(...){
	
	tss=PCB::running->ss;
	tbp=PCB::running->bp;
	tsp=PCB::running->sp;
	
	asm { 
		mov sp, tsp 
		mov bp, tbp
		mov ss, tss
	}
	
	
	system_mode=0;
	if(timerFlag==1){
		timerFlag=0;
		dispatch();
	}
}

void interrupt timer(){
	
	if (!RCS){
		tick();
		brojac--;
		PCB::sq->decrement();
	}
	
	if(system_mode==0){
		
		if (brojac == 0 || RCS == 1) {
	
			asm { 
				mov tsp, sp 
				mov tbp, bp
				mov tss, ss
			}
		
			PCB::running->ss = tss;
			PCB::running->sp = tsp;
			PCB::running->bp = tbp;
			
			if(PCB::running->status==AVAILABLE) Scheduler::put(PCB::running);			
			PCB::running = Scheduler::get();
			if(PCB::running==0) PCB::running=Idle::pcb;
	
			if(PCB::running!=0){
				tsp = PCB::running->sp;
				tss = PCB::running->ss; 
				tbp = PCB::running->bp;
				
				brojac = PCB::running->timeToLive;

				asm {
					mov sp, tsp  
					mov ss, tss
					mov bp, tbp
				}
			}     
		} 
		if(RCS==0) asm int 60h;
		RCS=0;		
	}else timerFlag=1;
	
}

unsigned oldTimerOFF, oldTimerSEG;

void timerInit(){
	asm{
		cli
		push es
		push ax

		mov ax,0   
		mov es,ax

		mov ax, word ptr es:0022h 
		mov word ptr oldTimerSEG, ax	
		mov ax, word ptr es:0020h	
		mov word ptr oldTimerOFF, ax	

		mov word ptr es:0022h, seg timer	 
		mov word ptr es:0020h, offset timer 

		mov ax, oldTimerSEG	 
		mov word ptr es:0182h, ax 
		mov ax, oldTimerOFF
		mov word ptr es:0180h, ax

		pop ax
		pop es
		sti
	}
}

void timerRestore(){
	asm {
		cli
		push es
		push ax

		mov ax,0
		mov es,ax

		mov ax, word ptr oldTimerSEG
		mov word ptr es:0022h, ax
		mov ax, word ptr oldTimerOFF
		mov word ptr es:0020h, ax

		pop ax
		pop es
		sti
	}
}