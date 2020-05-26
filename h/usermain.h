#ifndef _USERMAIN_H_
#define _USERMAIN_H_

class UserMain{
public:
	static int returnValue;
	static volatile int endOfMain;
	static void execute(int,char**);
};

#endif