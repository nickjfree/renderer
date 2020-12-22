#include "Mutex.h"
#include <stdio.h>

Mutex::Mutex()
{
	InitializeCriticalSection(&section);
}


Mutex::~Mutex()
{
}


void Mutex::Acquire() {
	EnterCriticalSection(&section);
}

void Mutex::Release() {
	LeaveCriticalSection(&section);
}
