#ifndef __CREATION_QUEUE__
#define __CREATION_QUEUE__


namespace Render{


// copy  cmd

template <class T>
class CCreationQueue
{
private:
	CRITICAL_SECTION m_QueueLock;
public:
	T * m_BatchHead;
	T * m_BatchRear;

public:
	int AddCreationBatch(T * Batch);
	T * RemoveBatch();
	int Lock();
	int UnLock();
	bool IsEmpty();
	CCreationQueue();
};

template <class T>
CCreationQueue<T>::CCreationQueue():m_BatchHead(0),m_BatchRear(0)
{
	InitializeCriticalSection(&m_QueueLock);
}

template <class T>
int CCreationQueue<T>::AddCreationBatch(T *Batch)
{
	EnterCriticalSection(&m_QueueLock);
	if(!m_BatchHead && !m_BatchRear)
	{
		m_BatchHead = m_BatchRear = Batch;
	}
	else
	{
		m_BatchRear->NextBatch = Batch;
		m_BatchRear = Batch;
	}
	Batch->NextBatch = NULL;
	LeaveCriticalSection(&m_QueueLock);
	return 0;
}

template <class T>
int CCreationQueue<T>::Lock()
{
	EnterCriticalSection(&m_QueueLock);
	return 0;
}

template <class T>
int CCreationQueue<T>::UnLock()
{
	LeaveCriticalSection(&m_QueueLock);
	return 0;
}

template <class T>
T * CCreationQueue<T>::RemoveBatch()
{
	T * Batch;
	EnterCriticalSection(&m_QueueLock);
	if(!m_BatchHead && !m_BatchRear)
	{
		Batch = NULL;
	}
	else
	{
		Batch = m_BatchHead;
		m_BatchHead = m_BatchHead->NextBatch;
		if(!m_BatchHead)
		{
			m_BatchRear = NULL;
		}
	}
	LeaveCriticalSection(&m_QueueLock);
	return Batch;
}

template <class T>
bool CCreationQueue<T>::IsEmpty()
{
	bool ret = false;
	EnterCriticalSection(&m_QueueLock);
	if(!m_BatchHead && !m_BatchRear)
	{
		ret = true;
	}
	LeaveCriticalSection(&m_QueueLock);
	return ret;
}


}//end namespace

#endif
