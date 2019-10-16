#include "Semaphore.h"



Semaphore::Semaphore(int MaxCount) {
	semaphore = CreateSemaphore(NULL, 0, MaxCount, NULL);
}


Semaphore::~Semaphore() {
	CloseHandle(semaphore);
}


void Semaphore::Acquire() {
	WaitForSingleObject(semaphore, -1);
}

void Semaphore::Release() {
	ReleaseSemaphore(semaphore, 1, 0);
}