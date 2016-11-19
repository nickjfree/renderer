#include "ConsoleTask.h"
#include <stdio.h>

USING_ALLOCATER(ConsoleTask);
USING_RECYCLE(ConsoleTask);

ConsoleTask::ConsoleTask() {
}


ConsoleTask::~ConsoleTask() {
}

int ConsoleTask::Work() {
	printf("console>");
	fgets(buffer, DEBUG_BUFFER_SIZE, stdin);
	return 0;
}

int ConsoleTask::Complete() {
	Scripting->RunDebug(buffer);
	return 0;
}
