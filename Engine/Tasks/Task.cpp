#include "Task.h"

USING_ALLOCATER(Task);
USING_RECYCLE(Task);

Task::Task()
{
    TaskList.Owner = this;
}


Task::~Task()
{
}

// work, called in worker thread
int Task::Work() {
    return 0;
}
// complete called in main thread
int Task::Complete() {
    return 0;
}
// failed called in main thread
int Task::Failed() {
    return 0;
}
