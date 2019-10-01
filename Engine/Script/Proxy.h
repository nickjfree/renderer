#ifndef __LUA_PROXY__
#define __LUA_PROXY__

#include "LuaStack.h"
#include "../Core/Object.h"
//#include <typeinfo.h>

template <typename T, typename FuncType = T>
struct MemberFunctor;


template <typename T>
struct ParamType {
    typedef T Type;
};


template <typename T>
struct ParamType<T&> {
    typedef T Type;
};

template <typename T>
struct ParamType<const T&> {
    typedef T Type;
};


// void ()
template <typename ClassType, typename FuncType>
struct MemberFunctor<void (ClassType::*)(), FuncType> {
    static int Call(lua_State * L, FuncType mfp) {
        ClassType* object;
        LuaStack::Get(L, 1, object);
        //		printf("calling funcs\n");
        (object->*mfp)();
        return 0;
    }
};


//R 
template <typename ClassType, typename R, typename FuncType>
struct MemberFunctor<R(ClassType::*)(), FuncType> {
    static int Call(lua_State * L, FuncType mfp) {
        ClassType* object;
        //ParamType<R>::Type ret;
        LuaStack::Get(L, 1, object);
        //		printf("calling funcs\n");
                // ret = (object->*mfp)();
                //  push return values
        LuaStack::Push(L, (object->*mfp)());
        return 1;
    }
};

//R () const
template <typename ClassType, typename R, typename FuncType>
struct MemberFunctor<R(ClassType::*)() const, FuncType> {
    static int Call(lua_State * L, FuncType mfp) {
        ClassType* object;
        //ParamType<R>::Type ret;
        LuaStack::Get(L, 1, object);
        //		printf("calling funcs\n");
                // ret = (object->*mfp)();
                //  push return values
        LuaStack::Push(L, (object->*mfp)());
        return 1;
    }
};

//void P1
template <typename ClassType, typename P1, typename FuncType>
struct MemberFunctor<void (ClassType::*)(P1), FuncType> {
    static int Call(lua_State * L, FuncType mfp) {
        ParamType<P1>::Type p1;
        ClassType* object;
        LuaStack::Get(L, 2, p1);
        LuaStack::Get(L, 1, object);
        //		printf("calling funcs %s\n", typeid(P1).name());
        (object->*mfp)(p1);
        //  push return values
        return 0;
    }
};

//void P1 P2
template <typename ClassType, typename P1, typename P2, typename FuncType>
struct MemberFunctor<void (ClassType::*)(P1, P2), FuncType> {
    static int Call(lua_State * L, FuncType mfp) {
        ParamType<P1>::Type p1;
        ParamType<P2>::Type p2;
        ClassType* object;
        LuaStack::Get(L, 3, p2);
        LuaStack::Get(L, 2, p1);
        LuaStack::Get(L, 1, object);
        //		printf("calling funcs %s\n", typeid(P1).name());
        (object->*mfp)(p1, p2);
        //  push return values
        return 0;
    }
};

//R P1
template <typename ClassType, typename R, typename P1, typename FuncType>
struct MemberFunctor<R(ClassType::*)(P1), FuncType> {
    static int Call(lua_State * L, FuncType mfp) {
        ParamType<P1>::Type p1;
        //ParamType<R>::Type ret;
        ClassType* object;
        LuaStack::Get(L, 2, p1);
        LuaStack::Get(L, 1, object);
        //		printf("calling funcs %s\n", typeid(P1).name());
                //ret = (object->*mfp)(p1);
                //  push return values
        LuaStack::Push(L, (object->*mfp)(p1));
        return 1;
    }
};


//R P1 P2
template <typename ClassType, typename R, typename P1, typename P2, typename FuncType>
struct MemberFunctor<R(ClassType::*)(P1, P2), FuncType> {
    static int Call(lua_State * L, FuncType mfp) {
        ParamType<P1>::Type p1;
        ParamType<P2>::Type p2;
        //ParamType<R>::Type ret;
        ClassType* object;
        LuaStack::Get(L, 3, p2);
        LuaStack::Get(L, 2, p1);
        LuaStack::Get(L, 1, object);
        //		printf("calling funcs %s\n", typeid(P1).name());
        //ret = (object->*mfp)(p1, p2);
        //  push return values
        LuaStack::Push(L, (object->*mfp)(p1, p2));
        return 1;
    }
};

//R P1 P2 P3
template <typename ClassType, typename R, typename P1, typename P2, typename P3, typename FuncType>
struct MemberFunctor<R(ClassType::*)(P1, P2, P3), FuncType> {
	static int Call(lua_State* L, FuncType mfp) {
		ParamType<P1>::Type p1;
		ParamType<P2>::Type p2;
		ParamType<P3>::Type p3;
		//ParamType<R>::Type ret;
		ClassType* object;
		LuaStack::Get(L, 4, p3);
		LuaStack::Get(L, 3, p2);
		LuaStack::Get(L, 2, p1);
		LuaStack::Get(L, 1, object);
		//		printf("calling funcs %s\n", typeid(P1).name());
		LuaStack::Push(L, (object->*mfp)(p1, p2, p3));
		return 1;
	}
};



template <typename FuncType>
int Proxy_Method(lua_State * L, FuncType mfp)
{
    return MemberFunctor<FuncType>::Call(L, mfp);
}


#define BEGIN_PROXY(class_name) \
	class Proxy_##class_name { \
		private: \
			static luaL_Reg Functions[]; \
		public: \
			static void Register(lua_State * L) { \
				int exists = luaL_newmetatable(L, #class_name); \
				luaL_setfuncs(L, Functions, 0); \
				lua_pushvalue(L, -1); \
				lua_setfield(L, -1, "__index"); \
				lua_setglobal(L, #class_name); \
			} \
			\
			static int Collect(lua_State * L) { \
				luaL_checktype(L , 1, LUA_TTABLE); \
			    lua_getfield(L, 1, "__self"); \
				Object * obj = *(Object **)lua_touserdata(L, -1); \
				if (obj->GetObjectId() != -1) { \
					obj->SetObjectId(-1); \
					obj->DecRef(); \
				} \
				return 0; \
			}



#define METHOD(method_name, member_func) \
	static int method_name (lua_State * L) {  \
		return Proxy_Method(L, member_func); \
	}



#define END_PROXY()   };


#define EXPORT_CLASS(class_name) \
	luaL_Reg Proxy_##class_name::Functions[] = {


#define EXPORT(class_name, function_name) \
	{#function_name, &Proxy_##class_name::##function_name},

#define GC(class_name) \
	{"__gc", &Proxy_##class_name::Collect},

#define END_EXPORT() {NULL, NULL}};

#define REGISTER_CLASS(L, class_name)  Proxy_##class_name::Register(L);


template <typename FuncType>
int TestCall(FuncType mfp)
{
    MemberFunctor<FuncType>::Call(mfp);
    return 0;
}

#endif