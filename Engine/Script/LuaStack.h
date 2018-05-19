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

private:
	// object increment id in global weakref table
	static unsigned int object_id;

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

	static void Push(lua_State* vm, bool value) {
		lua_pushboolean(vm, value);
	}

	static void Push(lua_State* vm, String& value) {
		lua_pushstring(vm, value);
	}

	static void Push(lua_State* vm, Vector3& value) {
		lua_newtable(vm);
		lua_pushnumber(vm, value.x);
		lua_setfield(vm, -2, "x");
		lua_pushnumber(vm, value.y);
		lua_setfield(vm, -2, "y");
		lua_pushnumber(vm, value.z);
		lua_setfield(vm, -2, "z");
	}

	static void Push(lua_State* vm, Quaternion& value) {
		lua_newtable(vm);
		lua_pushnumber(vm, value.x);
		lua_setfield(vm, -2, "x");
		lua_pushnumber(vm, value.y);
		lua_setfield(vm, -2, "y");
		lua_pushnumber(vm, value.z);
		lua_setfield(vm, -2, "z");
		lua_pushnumber(vm, value.w);
		lua_setfield(vm, -2, "w");
	}

	/*
		push object to stack.
		store a weak_ref in global object tables
	*/
	template <typename T>
	static void Push(lua_State* vm, T& value) {
		int id = value.GetObjectId();
		lua_getglobal(vm, "objects");
		if (id == -1) {
			// allocate a new id and export object
			id = object_id++;
			String& Name = value.GetTypeName();
			// export it
			lua_newtable(vm);
			int exists = luaL_newmetatable(vm, Name);
			lua_setmetatable(vm, -2);
			// set a userdata to __self field
			void * user_data = lua_newuserdata(vm, sizeof(void*));
			*(T **)user_data = &value;
			lua_setfield(vm, -2, "__self");
			// set the table to global table
			lua_seti(vm, -2, id);
			// inc refcount
			value.AddRef();
		}
		// push table "T" on stack, then remove global table
		lua_geti(vm, -1, id);
		lua_remove(vm, -2);
		value.SetObjectId(id);
		return;
	}

	template <typename T>
	static void Push(lua_State* vm, T* value) {
		int id = value->GetObjectId();
		lua_getglobal(vm, "objects");
		if (id == -1) {
			// allocate a new id and export object
			id = object_id++;
			String& Name = value->GetTypeName();
			// export it
			lua_newtable(vm);
			int exists = luaL_newmetatable(vm, Name);
			lua_setmetatable(vm, -2);
			// set a userdata to __self field
			void * user_data = lua_newuserdata(vm, sizeof(void*));
			*(T **)user_data = value;
			lua_setfield(vm, -2, "__self");
			// set the table to global table
			lua_seti(vm, -2, id);
			// inc refcount
			value->AddRef();
		}
		// push table "T" on stack, then remove global table
		lua_geti(vm, -1, id);
		lua_remove(vm, -2);
		value->SetObjectId(id);
	}

	//exports
	template <typename T>
	static void Export(lua_State* vm, T& value) {
		int id = value.GetObjectId();
		lua_getglobal(vm, "objects");
		if (id == -1) {
			// allocate a new id and export object
			id = object_id++;
			String& Name = value.GetTypeName();
			// export it
			lua_newtable(vm);
			int exists = luaL_newmetatable(vm, Name);
			lua_setmetatable(vm, -2);
			// set a userdata to __self field
			void * user_data = lua_newuserdata(vm, sizeof(void*));
			*(T **)user_data = &value;
			lua_setfield(vm, -2, "__self");
			// set table to entities
			lua_getglobal(vm, "entities");
			lua_pushvalue(vm, -2);
			lua_seti(vm, -2, id);
			lua_pop(vm, 1);
			// set the table to global table
			lua_seti(vm, -2, id);
			// add ref
			value.AddRef();
		}
		// pop global table
		lua_pop(vm, 1);
		value.SetObjectId(id);
		return;
	}

	template <typename T>
	static void Export(lua_State* vm, T* value) {
		int id = value->GetObjectId();
		lua_getglobal(vm, "objects");
		if (id == -1) {
			// allocate a new id and export object
			id = object_id++;
			String& Name = value->GetTypeName();
			// export it
			lua_newtable(vm);
			int exists = luaL_newmetatable(vm, Name);
			lua_setmetatable(vm, -2);
			// set a userdata to __self field
			void * user_data = lua_newuserdata(vm, sizeof(void*));
			*(T **)user_data = value;
			lua_setfield(vm, -2, "__self");
			// set table to entities
			lua_getglobal(vm, "entities");
			lua_pushvalue(vm, -2);
			lua_seti(vm, -2, id);
			lua_pop(vm, 1);
			// set the table to global table
			lua_seti(vm, -2, id);
			// add ref
			value->AddRef();
		}
		// pop global table
		lua_pop(vm, 1);
		value->SetObjectId(id);
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
		value = (unsigned int)luaL_checkinteger(vm, index);
	}

	static void Get(lua_State* vm, int index, int& value) {
		value = (int)luaL_checkinteger(vm, index);
	}

	static void Get(lua_State* vm, int index, char *& value) {
		value = (char*)luaL_checkstring(vm, index);
	}

	static void Get(lua_State* vm, int index, float & value) {
		value = (float)luaL_checknumber(vm, index);
	}

	static void Get(lua_State* vm, int index, String& value) {
		value = (char*)luaL_checkstring(vm, index);
	}

	static void Get(lua_State* vm, int index, Vector3& value) {
		float x, y, z;
		luaL_checktype(vm, index, LUA_TTABLE);
		lua_getfield(vm, index, "x");
		x = luaL_checknumber(vm, -1);
		lua_getfield(vm, index, "y");
		y = luaL_checknumber(vm, -1);
		lua_getfield(vm, index, "z");
		z = luaL_checknumber(vm, -1);
		lua_pop(vm, 3);
		value = Vector3(x, y, z);
	}

	static void Get(lua_State* vm, int index, Quaternion& value) {
		float x, y, z, w;
		luaL_checktype(vm, index, LUA_TTABLE);
		lua_getfield(vm, index, "x");
		x = luaL_checknumber(vm, -1);
		lua_getfield(vm, index, "y");
		y = luaL_checknumber(vm, -1);
		lua_getfield(vm, index, "z");
		z = luaL_checknumber(vm, -1);
		lua_getfield(vm, index, "w");
		w = luaL_checknumber(vm, -1);
		value = Quaternion();
		value.x = x;
		value.y = y;
		value.z = z;
		value.w = w;
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
		luaL_checktype(vm, index, LUA_TTABLE);
		lua_getfield(vm, index, "__self");
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