#ifndef __SEMAPHORE__
#define __SEMAPHORE__

#include <Windows.h>


class Semaphore {
private:
    HANDLE semaphore;
public:
    // acquire 
    void Acquire();
    // release
    void Release();
    Semaphore(int MaxCount);
    virtual ~Semaphore();
};

#endif 
