#include "ThreadLocal.h"


DWORD ThreadLocal::slot = -1;

ThreadLocal::ThreadLocal() {
	slot = TlsAlloc();
}


ThreadLocal::~ThreadLocal() {
	TlsFree(slot);
}


void * ThreadLocal::GetThreadLocal() {
	return TlsGetValue(slot);
}

bool ThreadLocal::SetThreadLocal(void * value) {
	return TlsSetValue(slot, value);
}