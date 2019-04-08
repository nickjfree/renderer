#include "ObjectContainer.h"

ObjectContainer::ObjectContainer(void) :m_FirstPool(0), m_FreeItem(0), m_ObjectSize(0), m_ItemSize(0), m_MemUsed(0), m_PoolNum(0), m_UsedItemNum(0), m_UsedItem(0)
{
}

ObjectContainer::ObjectContainer(int ObjectSize) : m_FirstPool(0), m_FreeItem(0), m_ObjectSize(0), m_ItemSize(0), m_MemUsed(0), m_PoolNum(0), m_UsedItemNum(0), m_UsedItem(0)
{
    Init(ObjectSize);
}

ObjectContainer::~ObjectContainer(void)
{
}

int ObjectContainer::Init(int ObjectSize)
{
    PoolHeader * Pool = m_FirstPool;
    m_ObjectSize = ObjectSize + 16 - 1;
    m_ItemSize = m_ObjectSize + sizeof(ItemHeader) + sizeof(void*);
    int PoolSizeInByte = POOL_SIZE * m_ItemSize + sizeof(PoolHeader);
    Pool = (PoolHeader*)new BYTE[PoolSizeInByte];
    memset(Pool, 0, PoolSizeInByte);
    Pool->base = 0;
    MakeFreePool(Pool);
    return 0;
}

int ObjectContainer::MakeFreePool(PoolHeader * Pool)
{
    Pool->data = (char*)Pool + sizeof(PoolHeader);
    Pool->next_pool = NULL;
    ItemHeader * Header = (ItemHeader*)Pool->data;
    for (int i = 0; i < POOL_SIZE; i++)
    {
        Header->id = Pool->base + i;
        Header->data = 0;
        Header->next_item = NULL;
        Header->next_used = NULL;
        Header->pre_used = NULL;
        Header->data = (char*)Header + sizeof(ItemHeader) + sizeof(void*);
        //unsigned long long offset = (unsigned long long)(16 - (unsigned long long)Header->data) & 0x0F;
        //Header->data = (void*)((unsigned long long)Header->data + offset);  // aligned 16 byte
        *(unsigned long long*)((char *)Header->data - sizeof(void*)) = (unsigned long long)Header;
        ToFreeItem(Header);
        Header = (ItemHeader*)((char*)Header + m_ItemSize);
    }
    PoolHeader * PoolAdd = m_FirstPool;
    if (PoolAdd)
    {
        while (PoolAdd->next_pool)
        {
            PoolAdd = PoolAdd->next_pool;
        }
        PoolAdd->next_pool = Pool;
    }
    else
    {
        m_FirstPool = Pool;
    }
    m_PoolNum++;
    int PoolSizeInByte = POOL_SIZE * m_ItemSize + sizeof(PoolHeader);
    m_MemUsed += PoolSizeInByte;
    return 0;
}

int ObjectContainer::ToFreeItem(ItemHeader * Header)
{
    if (Header->next_used && Header->pre_used)
    {
        Header->pre_used->next_used = Header->next_used;
        Header->next_used->pre_used = Header->pre_used;
    }
    if (Header == m_UsedItem)
    {
        m_UsedItem = Header->next_used;
    }
    Header->next_item = m_FreeItem;
    m_FreeItem = Header;
    return 0;
}

int ObjectContainer::AllocOne(void ** pData)
{
    int id = -1;
    if (!m_FreeItem)
    {
        // need a new pool
        PoolHeader * Pool;
        int PoolSizeInByte = POOL_SIZE * m_ItemSize + sizeof(PoolHeader);
        Pool = (PoolHeader*)new BYTE[PoolSizeInByte];
        memset(Pool, 0, PoolSizeInByte);
        Pool->base = m_PoolNum * POOL_SIZE;
        MakeFreePool(Pool);
    }
    ItemHeader * Header = m_FreeItem;
    *pData = Header->data;
    id = Header->id;
    m_FreeItem = m_FreeItem->next_item;
    ToUsedItem(Header);
    m_UsedItemNum++;
    return id;
}

int ObjectContainer::ReleaseItem(ItemHeader * Header)
{
    ToFreeItem(Header);
    return 0;
}

void * ObjectContainer::GetObjectByID(int Id)
{
    ItemHeader * Header;
    Header = GetItem(Id);
    return Header->data;
}

int ObjectContainer::ReleaseOne(int Id)
{
    ItemHeader * Header = GetItem(Id);
    ToFreeItem(Header);
    m_UsedItemNum--;
    if (m_UsedItemNum == 0)
    {
        m_UsedItem = NULL;
    }
    return 0;
}

ItemHeader * ObjectContainer::GetItem(int id)
{
    int FindID = id;
    if (!m_FirstPool)
        return NULL;
    PoolHeader * Pool = m_FirstPool;
    while (FindID >= POOL_SIZE)
    {
        FindID -= POOL_SIZE;
        Pool = Pool->next_pool;
    }
    ItemHeader * Header = (ItemHeader*)Pool->data;
    Header = (ItemHeader*)((char*)Header + FindID * m_ItemSize);
    return Header;
}

int ObjectContainer::Destroy(void)
{
    //Free all pools
    return 0;
}

int ObjectContainer::ToUsedItem(ItemHeader * Header)
{
    Header->next_item = NULL;
    if (m_UsedItem)
    {
        Header->next_used = m_UsedItem->next_used;
        Header->pre_used = m_UsedItem;
        m_UsedItem->next_used->pre_used = Header;
        m_UsedItem->next_used = Header;
    }
    else
    {
        Header->next_used = Header->pre_used = Header;
        m_UsedItem = Header;
    }
    return 0;
}

void * ObjectContainer::GetNext(void * Object)
{
    ItemHeader * Header;
    if (!m_UsedItemNum)
    {
        return NULL;
    }
    if (!Object)
    {
        return m_UsedItem->data;
    }
    Header = *(ItemHeader**)((char*)Object - sizeof(void*));
    //Header = (ItemHeader*)((char*)Object - sizeof(ItemHeader));
    Header = Header->next_used;
    if (Header == m_UsedItem)
    {
        return NULL;
    }
    return Header->data;
}
