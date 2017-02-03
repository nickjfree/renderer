#ifndef __WORK_QUEUE__
#define __WORK_QUEUE__

/*
	Work queue
*/
#include "Core\System.h"
#include "Container\LinkList.h"
#include "Container\Vector.h"
#include "Task.h"
#include "Mutex.h"
#include "ThreadLocal.h"

class WorkerThread;

class WorkQueue : public System
{
	BASEOBJECT(System);
	OBJECT(WorkQueue);
private:
	// pending tasks
	LinkList<Task> PendingTasks;
	// finished task. both complete and failed
	LinkList<Task> FinishTasks;
	// pending queue mutex
	Mutex Pending;
	// finish queue mute
	Mutex Finish;
	// worker threads
	Vector<WorkerThread*> Workers;
	// thread local
	ThreadLocal * tls;
private:
	void HandleComplete(int Limit=500);
public:
	WorkQueue(Context * context);
	~WorkQueue();
	// add task to queue
	int QueueTask(Task * task);
	// get pending task
	Task * GetPendingTask();
	// get finish tasks
	Task * GetFinishTask();
	// complete task
	int CompleteTask(Task * task);
	// on complete
	virtual int Update(int ms);
	// init
	virtual int Initialize();
};

#endif