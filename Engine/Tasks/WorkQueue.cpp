#include "WorkQueue.h"
#include "WorkerThread.h"
#include "ThreadLocal.h"


WorkQueue::WorkQueue(Context * context) : System(context)
{
}


WorkQueue::~WorkQueue()
{
}

// add task to queue
int WorkQueue::QueueTask(Task * task) {
	Pending.Acquire();
	task->TaskList.InsertAfter(&PendingTasks);
	Pending.Release();
	return 0;
}


int WorkQueue::CompleteTask(Task * task) {
	Finish.Acquire();
	task->TaskList.InsertAfter(&FinishTasks);
	Finish.Release();
	return 0;
}

Task * WorkQueue::GetPendingTask() {
	Task * task = NULL;
	Pending.Acquire();
	LinkList<Task>::Iterator Iter = PendingTasks.Last();
	if (Iter != PendingTasks.End()) {
		task = *Iter;
		task->TaskList.Remove();
	}
	Pending.Release();
	return task;
}

Task * WorkQueue::GetFinishTask() {
	Task * task = NULL;
	Finish.Acquire();
	LinkList<Task>::Iterator Iter = FinishTasks.Last();
	if (Iter != FinishTasks.End()) {
		task = *Iter;
		task->TaskList.Remove();
	}
	Finish.Release();
	return task;
}

int WorkQueue::Initialize() {
	// init thread index for main thread
	tls = new ThreadLocal();
	ThreadLocal::SetThreadLocal(0);
	// start threads
	int cores = 2;
	for (int i = 0; i < cores; i++) {
		WorkerThread * Thread = new WorkerThread(this, i+1);
		Workers.PushBack(Thread);
		Thread->Start();
	}
	return 0;
}

int WorkQueue::Update(int ms) {
	HandleComplete();
	return 0;
}

void WorkQueue::HandleComplete(int Limit) {
		// handle all complete tasks
	int Handled = 0;
	while (Handled < Limit) {
		Task * task = GetFinishTask();
		if (!task) {
			return;
		}
		// call complete
		task->Complete();
		task->Recycle();
		Handled++;
	}
}
