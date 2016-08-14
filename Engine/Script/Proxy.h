#ifndef __LUA_PROXY__
#define __LUA_PROXY__

#include "LuaStack.h"
#include <typeinfo.h>

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


template <typename ClassType, typename R, typename P1, typename FuncType>
struct MemberFunctor<R (ClassType::*)(P1), FuncType> {
	static void Call(FuncType mfp) {
		ParamType<P1>::Type p1;
		LuaStack::Pop(NULL, p1);
		printf("calling funcs %s\n", typeid(P1).name());
	}
};


template <typename FuncType>
int TestCall(FuncType mfp)
{
	MemberFunctor<FuncType>::Call(mfp);
	return 0;
}

#endif