#ifndef __LUA_PROXY__
#define __LUA_PROXY__

#include "LuaStack.h"
#include "../Core/Object.h"

template <typename T, typename FuncType = T>
struct MemberFunctor;


// strip types
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


// get parameters
template <typename T>
auto GetParameter(lua_State * L, int Index)
{
	ParamType<T>::Type parameter;
	LuaStack::Get(L, Index, parameter);
	return parameter;
}

/*
	template for  void func(p...) and r func(p...)
*/
template <typename ClassType, typename MemberFunction, typename ... Parameter>
struct WithParameters {

	template<std::size_t ... Index>
	static int VoidCall(lua_State* L, MemberFunction mfp, std::index_sequence<Index...>) {
		// get objects
		ClassType* object;
		LuaStack::Get(L, 1, object);
		// call function
		(object->*mfp)(GetParameter<Parameter>(L, Index+2)...);
		return 0;
	}
	template<std::size_t ... Index>
	static int ReturnCall(lua_State* L, MemberFunction mfp, std::index_sequence<Index...>) {
		// get objects
		ClassType* object;
		LuaStack::Get(L, 1, object);
		// call function
		LuaStack::Push(L, (object->*mfp)(GetParameter<Parameter>(L, Index+2)...));
		return 1;
	}
};

// void ()
template <typename ClassType, typename FuncType>
struct MemberFunctor<void (ClassType::*)(), FuncType> {
	static int Call(lua_State* L, FuncType mfp) {
		ClassType* object;
		LuaStack::Get(L, 1, object);
		(object->*mfp)();
		return 0;
	}
};

// void func(p...)
template <typename ClassType, typename ...Paramter, typename FuncType>
struct MemberFunctor<void (ClassType::*)(Paramter...), FuncType> {
	static int Call(lua_State* L, FuncType mfp) {
		return WithParameters<ClassType, FuncType, Paramter...>::VoidCall(L, mfp, std::make_index_sequence<sizeof...(Paramter)>{});
	}
};


//  return func()
template <typename ClassType, typename R, typename FuncType>
struct MemberFunctor<R(ClassType::*)(), FuncType> {
	static int Call(lua_State* L, FuncType mfp) {
		ClassType* object;
		LuaStack::Get(L, 1, object);
		LuaStack::Push(L, (object->*mfp)());
		return 1;
	}
};

// return func(p...)
template <typename ClassType, typename R, typename ...Paramter, typename FuncType>
struct MemberFunctor<R(ClassType::*)(Paramter...), FuncType> {
	static int Call(lua_State* L, FuncType mfp) {
		return WithParameters<ClassType, FuncType, Paramter...>::ReturnCall(L, mfp, std::make_index_sequence<sizeof...(Paramter)>{});
	}
};

// return func() const
template <typename ClassType, typename R, typename FuncType>
struct MemberFunctor<R(ClassType::*)() const, FuncType> {
	static int Call(lua_State* L, FuncType mfp) {
		ClassType* object;
		LuaStack::Get(L, 1, object);
		LuaStack::Push(L, (object->*mfp)());
		return 1;
	}
};

// return func(p...) const
template <typename ClassType, typename R, typename ...Paramter, typename FuncType>
struct MemberFunctor<R(ClassType::*)(Paramter...) const, FuncType> {
	static int Call(lua_State* L, FuncType mfp) {
		return WithParameters<ClassType, FuncType, Paramter...>::ReturnCall(L, mfp, std::make_index_sequence<sizeof...(Paramter)>{});
	}
};


template <typename FuncType>
int Proxy_Method(lua_State* L, FuncType mfp)
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