#ifndef __OBCONTAINER__
#define __OBCONTAINER__

#include <windows.h>


#define POOL_SIZE 100

typedef struct _PoolHeader
{
	_PoolHeader * next_pool;
	int          base;
	void *       data;
}PoolHeader;

typedef struct _ItemHeader
{
	_ItemHeader * next_item;
	_ItemHeader * next_used;
	_ItemHeader * pre_used;
	int          id;
	void *       data;
}ItemHeader;

class CObjectContainer
{
private:
	int m_ObjectSize;
	int m_ItemSize;      // object size + header size
	PoolHeader * m_FirstPool;
	ItemHeader * m_FreeItem;
	ItemHeader * m_UsedItem;
	DWORD        m_MemUsed;
	int          m_PoolNum;
	int          m_UsedItemNum;
public:
	CObjectContainer(void);
	CObjectContainer(int ObjectSize);
	~CObjectContainer(void);
	int Init(int ObjectSize);
private:
	int MakeFreePool(PoolHeader * Pool);
	int ReleaseItem(ItemHeader * Header);
	ItemHeader * GetItem(int id);
	int ToFreeItem(ItemHeader * Header);
public:
	int AllocOne(void ** pData);
	void * GetObjectByID(int Id);
	int ReleaseOne(int Id);
	int Destroy(void);
	int ToUsedItem(ItemHeader * Header);
	void * GetNext(void * Object);
};



#endif