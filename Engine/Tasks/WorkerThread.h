#ifndef __WORKER_THREAD__
#define __WORKER_THREAD__

/*
	worker thread for processing async tasks.
	Resource loading. animation and some physics
*/
#include <Windows.h>
#include "WorkQueue.h"

// win32 version

class WorkerThread
{
protected:
	// thread handle
	HANDLE hThread;
	// DWORD thread id
	DWORD ThreadId;
	// working flag
	bool Working;
	// thread index
	int Index;
	// task finish
	int TaskFinish;
	// Work queue
	WorkQueue* Queue;
public:
	static int WINAPI ThreadEntry(WorkerThread* Thread);
public:
	WorkerThread(WorkQueue* Queue, int Index);
	virtual ~WorkerThread();
	virtual void WorkFunction(void* Param);
	int Start();
	int Terminate();
	int Stop();
};

#endif