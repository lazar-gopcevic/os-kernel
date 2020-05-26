#ifndef _event_h_
#define _event_h_

typedef unsigned char IVTNo;

class KernelEv;

class Event {
public:
	Event (IVTNo ivtNo);
	~Event ();
	void wait ();
protected:
	friend class KernelEv;
	void signal(); // can call KernelEv
private:
	int id;
};

#include "kernel.h"

#define PREPAREENTRY(no, old) \
		void interrupt rout##no(...); \
		void interrupt (*intRout##no)(...) = &rout##no; \
		IVTEntry entry##no(0x##no, intRout##no); \
		void interrupt rout##no(...){ \
			if(old) entry##no.callOldRout(); \
			entry##no.signal(); \
		}

#endif
