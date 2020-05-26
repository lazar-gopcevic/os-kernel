#ifndef _PARAM_H_
#define _PARAM_H_

struct Parameters{
	unsigned char cParam;
	unsigned long ulParam[3];
	unsigned uiParam[3];
	int iParam[3];
	void* ptr[3];
	unsigned int function;
	unsigned int caller;
	int returnID;
	int reqID;
};

#endif