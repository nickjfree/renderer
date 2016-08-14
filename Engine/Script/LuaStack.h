#ifndef __LUA_STACK__
#define __LUA_STACK__

extern  "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
}

#include "Core/Str.h"


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

	static void Push(lua_State* vm, String value) {
		lua_pushstring(vm, value);
	}

	template <typename T>
	static void Push(lua_State* vm, T& value) {
		return;
	}

	template <typename T>
	static void Push(lua_State* vm, T value) {
		return;
	}

	template <typename T>
	static void Push(lua_State* vm, T* value) {
		return;
	}

	// gets

	static void Pop(lua_State* vm, unsigned int & value) {
		value = lua_tointeger(vm, -1);
		lua_pop(vm, 1);
	}

	static void Pop(lua_State* vm, int& value) {
		value = lua_tointeger(vm, -1);
		lua_pop(vm, 1);
	}

	static void Pop(lua_State* vm, char *& value) {
		value = (char*)lua_tostring(vm, -1);
		lua_pop(vm, 1);
	}

	static void Pop(lua_State* vm, float & value) {
		value = lua_tonumber(vm, -1);
		lua_pop(vm, 1);
	}

	static void Pop(lua_State* vm, String& value) {
		value = (char*)lua_tostring(vm, -1);
		lua_pop(vm, 1);
	}

	template <typename T>
	static void Pop(lua_State* vm, T& value) {
		return;
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