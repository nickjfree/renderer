#ifndef __MUTEX__
#define __MUTEX__

/*
	Win32 critical section
*/
#include <Windows.h>


class Mutex
{
private:
public:
	CRITICAL_SECTION section;
public:
	Mutex();
	~Mutex();
	void Acquire();
	void Release();
};

#endif 
