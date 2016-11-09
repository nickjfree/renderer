#ifndef __LUA_STACK__
#define __LUA_STACK__

extern  "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

#include "Core/Str.h"
#include "Math/LinearMath.h"


class LuaStack {

public:
	static void Push(lua_State* vm, unsigned int value) {
		lua_pushinteger(vm, value);
	}

	static void Push(lua_State* vm, int value) {
		lua_pushinteger(vm, value);
	}

	static void Push(lua_State* vm, char * value) {
		lua_pushstring(vm, value);
	}

	static void Push(lua_State* vm, float value) {
		lua_pushnumber(vm, value);
	}

	static void Push(lua_State* vm, String& value) {
		lua_pushstring(vm, value);
	}

	template <typename T>
	static void Push(lua_State* vm, T& value) {
		return;
	}

	template <typename T>
	static void Push(lua_State* vm, T* value) {
		return;
	}

	//// pops

	//static void Pop(lua_State* vm, unsigned int & value) {
	//	value = lua_tointeger(vm, -1);
	//	lua_pop(vm, 1);
	//}

	//static void Pop(lua_State* vm, int& value) {
	//	value = lua_tointeger(vm, -1);
	//	lua_pop(vm, 1);
	//}

	//static void Pop(lua_State* vm, char *& value) {
	//	value = (char*)lua_tostring(vm, -1);
	//	lua_pop(vm, 1);
	//}

	//static void Pop(lua_State* vm, float & value) {
	//	value = luaL_checknumber(vm, -1);
	//	lua_pop(vm, 1);
	//}

	//static void Pop(lua_State* vm, String& value) {
	//	value = (char*)lua_tostring(vm, -1);
	//	lua_pop(vm, 1);
	//}

	//template <typename T>
	//static void Pop(lua_State* vm, T& value) {
	//	return;
	//}

	// gets

	static void Get(lua_State* vm, int index, unsigned int & value) {
		value = (unsigned int)lua_tointeger(vm, -1);
	}

	static void Get(lua_State* vm, int index, int& value) {
		value = (int)lua_tointeger(vm, -1);
	}

	static void Get(lua_State* vm, int index, char *& value) {
		value = (char*)lua_tostring(vm, -1);
	}

	static void Get(lua_State* vm, int index, float & value) {
		value = (float)luaL_checknumber(vm, -1);
	}

	static void Get(lua_State* vm, int index, String& value) {
		value = (char*)lua_tostring(vm, -1);
	}

	template <typename T>
	static void Get(lua_State* vm, int index, T& value) {
		char * Name = T::GetTypeNameStatic();
		T * data = *(T **)luaL_checkudata(vm, index, Name);
		value = *data;
	}

	template <typename T>
	static void Get(lua_State* vm, int index, T* &value) {
		char * Name = T::GetTypeNameStatic();
		luaL_checktype(vm, 1, LUA_TTABLE);
		lua_getfield(vm, 1, "__self");
		T * data = *(T **)lua_touserdata(vm, -1);
		value = data;
	}

	//template <typename T>
	//static void Push(lua_State* vm, T value) {
	//	return;
	//}

	//template <typename T>
	//static void Push(lua_State* vm, T* value) {
	//	return;
	//}

};


#endif 