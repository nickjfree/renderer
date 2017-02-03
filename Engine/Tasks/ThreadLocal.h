#ifndef __THREAD_LOCAL__
#define __THREAD_LOCAL__

#include <Windows.h>
/*
	thread local. only use one slot
*/

class ThreadLocal {

private:
	static DWORD slot;
public:
	ThreadLocal();
	virtual ~ThreadLocal();
	// get thread local value
	static void * GetThreadLocal();
	// set thread local value
	static bool SetThreadLocal(void * value);
};

#endif 