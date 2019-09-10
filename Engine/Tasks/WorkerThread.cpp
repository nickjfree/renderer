#include "WorkerThread.h"
#include "ThreadLocal.h"


WorkerThread::WorkerThread(WorkQueue * Queue_, int index) : TaskFinish(0), Index(index), Queue(Queue_)
{
    // create thread in suspended mode
    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadEntry, this, CREATE_SUSPENDED, &ThreadId);
}


WorkerThread::~WorkerThread()
{
}

int WINAPI WorkerThread::ThreadEntry(WorkerThread * Thread) {
    // init thread local value
    ThreadLocal::SetThreadLocal(reinterpret_cast<void*>(Thread->Index));
    // call work function
    Thread->WorkFunction(NULL);
    return 0;
}


void WorkerThread::WorkFunction(void * Param) {
    while (1) {
        if (!Working) {
            // exit if not working
            break;
        }
        Task * task = Queue->GetPendingTask();
        if (task) {
            // call work function
            int failed = task->Work();
            // put to complte queue
            Queue->CompleteTask(task);
            TaskFinish++;
        }
        else {
            // no pending tasks. sleep to give cpu time to others
            Sleep(0);
        }
    }
    // clean stuff

}


int WorkerThread::Start() {
    Working = true;
    ResumeThread(hThread);
    return 0;
}


int WorkerThread::Terminate() {
    // TerminateThread(hThread, -1);
    return 0;
}

int WorkerThread::Stop() {
    Working = false;
    return 0;
}