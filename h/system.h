#ifndef _SYSTEM_H_
#define _SYSTEM_H_

void interrupt system_call(...);
void interrupt system_call_end(...);

void timerInit();
void interrupt timer();
void timerRestore();

#endif