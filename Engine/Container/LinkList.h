#ifndef __LINK_LIST__
#define __LINK_LIST__



template <class T>
class LinkList
{
public:
    T * Owner;
    LinkList * Prev;
    LinkList * Next;
    LinkList * Head;
    class Iterator {
    public:
        LinkList * ptr;
        bool operator == (Iterator& rh) {
            return ptr == rh.ptr;
        }
        bool operator != (Iterator& rh) {
            return ptr != rh.ptr;
        }
        Iterator& operator ++(int) {
            ptr = ptr->Next;
            return *this;
        }
        T* operator * () {
            return ptr->Owner;
        }
    };
public:
    LinkList() { Prev = Next = Head = this; };
    virtual ~LinkList() {};
    void InsertAfter(LinkList * Node) {
        Head = Node->Head;
        Next = Node->Next;
        Prev = Node;
        Node->Next->Prev = this;
        Node->Next = this;
    }
    void Remove() {
        Prev->Next = Next;
        Next->Prev = Prev;
        Next = Prev = Head = NULL;
    }

    Iterator Begin() {
        Iterator Iter;
        Iter.ptr = Head->Next;
        return Iter;
    }
    Iterator End() {
        Iterator Iter;
        Iter.ptr = Head;
        return Iter;
    }
    Iterator Last() {
        Iterator Iter;
        Iter.ptr = Head->Prev;
        return Iter;
    }
};

#endif
