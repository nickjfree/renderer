#ifndef __SHORTCUTS__
#define __SHORTCUTS__

#include "ObjectTypeId.h"

// macro defines

// custom memory allocater for special classes, override new and delete operator in header files
#define  DECLAR_ALLOCATER(Object) \
	private: \
		static Allocater<Object> m_Allocater; \
		static ObjectTypeId Class; \
	public: \
		void * operator new(size_t size){ return m_Allocater.Alloc(); } \
		void operator delete(void * ptr) { m_Allocater.Free(ptr); } \

// static allocater member in cpp file
#define USING_ALLOCATER(Object)  \
	Allocater<Object> Object::m_Allocater = {}; \
	ObjectTypeId Object::Class = ObjectTypeId(); \


// the obejct is used in a recyced pool, the free object is used again without desctruct and construct, this is a faster version of DECLAR_ALLOCATER
#define  DECLAR_RECYCLE(Object) \
	private: \
		static RecyclePool<Object> m_RecyclePool; \
	public: \
	static Object * Create(){ return m_RecyclePool.Create(); } \
	virtual void Recycle() { m_RecyclePool.Recycle(this); }

// static recyclepool in cpp file
#define USING_RECYCLE(Object)  \
	RecyclePool<Object> Object::m_RecyclePool = {};



// common object memebers and functions
#define OBJECT(TypeName) \
	public: \
		static String& GetTypeNameStatic() { static String TypeName(#TypeName); return TypeName; } \
		virtual String& GetTypeName() { return GetTypeNameStatic(); } \
		virtual String& GetBaseTypeName() { return GetBaseTypeNameStatic(); } \
	//	virtual unsigned int GetObjectId() { return ObjectId==-1?ObjectId=Class.GetId():ObjectId; };
		//operator int() { return (int)this; } \

#define UUID() \
	public: \
		virtual unsigned int GetObjectId() { return ObjectId==-1?ObjectId=Class.GetId():ObjectId; }; \

#define BASEOBJECT(TypeName) \
		public: \
			static String& GetBaseTypeNameStatic() { static String TypeName(#TypeName); return TypeName; } \


#define LOADEROBJECT(TypeName) \
		public: \
			static String& GetLoaderTypeNameStatic() { static String TypeName(#TypeName); return TypeName; } \
			virtual String& GetLoaderTypeName() { return GetLoaderTypeNameStatic(); }

#endif