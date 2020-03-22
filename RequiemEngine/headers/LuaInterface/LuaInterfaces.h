 #pragma once
#include <string>
#include <headers/Debug.h>
extern "C"
{
#include"lua.h"
#include "lauxlib.h"  
#include "lualib.h" 
}

//namespace lua_func
//{
#define LUA_VOID_FUNC( FUNC_NAME , TYPE ,FUNC )\
	int FUNC_NAME (lua_State *L)\
	{\
		lua_getfield(L, 1, "pdata");\
		TYPE *pthis = static_cast< TYPE *>(lua_touserdata(L, -1));\
		pthis->FUNC();\
		return 0;\
	}\

	template <typename T>
	static	void WrapUserData(lua_State* L, T& data)
	{
		lua_newtable(L);
		lua_pushlightuserdata(L, &data);
		lua_setfield(L, -1, "pdata");
	}

	template <typename T>
	static	T& UnWrapUserData(lua_State* L, int id)
	{
		lua_getfield(L, id, "pdata");
		T* data = reinterpret_cast<T*>(lua_touserdata(L, -1));
		lua_pop(L, 1);
		return *data;
	}

	template <typename T>
	static	T& UnWrapPUserData(lua_State* L, int id)
	{
		lua_getfield(L, id, "pdata");
		T* data = reinterpret_cast<T*>(lua_touserdata(L, -1));
		lua_pop(L, 1);
		return data;
	}

	template<typename T>
	static void PushLuaData(lua_State* L, T data);

	template<>
	static void PushLuaData(lua_State* L, int data)
	{
		lua_pushinteger(L, data);
	}

	template<>
	static void PushLuaData(lua_State* L, float data)
	{
		lua_pushnumber(L, data);
	}

	template<>
	static void PushLuaData(lua_State* L, double data)
	{
		lua_pushnumber(L, data);
	}

	template<>
	static void PushLuaData(lua_State* L, std::string const& data)
	{
		lua_pushstring(L, data.c_str());
	}


	template <typename T>
	static T GetLuaData(lua_State* L, int id);

	template<>
	static int GetLuaData(lua_State* L, int id)
	{
		return lua_tointeger(L, id);
	}

	template<>
	static uint32_t GetLuaData(lua_State* L, int id)
	{
		return static_cast<uint32_t>(lua_tointeger(L, id));
	}

	template<>
	static std::string GetLuaData(lua_State* L, int id)
	{
		return std::string(lua_tostring(L, id));
	}

	template<>
	static float GetLuaData(lua_State* L, int id)
	{
		return lua_tonumber(L, id);
	}

	template<>
	static bool GetLuaData(lua_State* L, int id)
	{
		return lua_toboolean(L, id);
	}

	template<int...Ints>
	struct int_pack {};

	template<int Begin, int Count, int...Tail>
	struct make_int_range_type {
		typedef typename make_int_range_type<Begin, Count - 1, Begin + Count - 1, Tail...>::type type;
	};

	template<int Begin, int...Tail>
	struct make_int_range_type<Begin, 0, Tail...> {
		typedef int_pack<Tail...> type;
	};

	template<int Begin, int Count>
	inline typename make_int_range_type<Begin, Count>::type
		make_int_range()
	{
		return typename make_int_range_type<Begin, Count>::type();
	}

	template<class MemFunPtrType, MemFunPtrType PMF>
	struct lua_mem_func_wrapper;

	template<class Clazz, class ReturnType, class...Args, ReturnType(Clazz:: * PMF)(Args...)>
	struct lua_mem_func_wrapper<ReturnType(Clazz::*)(Args...), PMF> {
		static int doit(lua_State* L) {
			return doit_impl(L, make_int_range<2, sizeof...(Args)>());
		}
	private:
		template<int...Indices>
		static int doit_impl(lua_State* L, int_pack<Indices...>) {
			PushLuaData<ReturnType>(L,
				(UnWrapPUserData<Clazz>(L, 1)->*PMF)(
					GetLuaData<Args>(L, Indices)...
					)
				);
			return 1;
		}
	};

	template<class FunPtrType, FunPtrType PF>
	struct lua_func_wrapper;

	template<class ReturnType, class...Args, ReturnType(*PF)(Args...)>
	struct lua_func_wrapper<ReturnType(*)(Args...), PF> {
		static int doit(lua_State* L) {
			return doit_impl(L, make_int_range<1, sizeof...(Args)>());
		}
	private:
		template<int...Indices>
		static int doit_impl(lua_State* L, int_pack<Indices...>) {
			PushLuaData<ReturnType>(L,
				(*PF)(
					GetLuaData<Args>(L, Indices)...
					)
				);
			return 1;
		}
	};

	template<class...Args, void(*PF)(Args...)>
	struct lua_func_wrapper<void(*)(Args...), PF> {
		static int doit(lua_State* L) {
			return doit_impl(L, make_int_range<1, sizeof...(Args)>());
		}
	private:
		template<int...Indices>
		static int doit_impl(lua_State* L, int_pack<Indices...>) {
				(*PF)(
					GetLuaData<Args>(L, Indices)...
				);
			return 0;
		}
	};

#define GET_MEM_FUN_WRAPPER(...) &lua_mem_func_wrapper<decltype(__VA_ARGS__),__VA_ARGS__>::doit
#define GET_FUN_WRAPPER(...) &lua_func_wrapper<decltype(__VA_ARGS__),__VA_ARGS__>::doit
//}

#define CHECK_LUAERR(L, STATE) {CheckLuaErr(L, STATE, __LINE__, __FILE__);}
static void CheckLuaErr(lua_State* L, int state, int line, std::string const& file)
{
	if (state != 0)
	{
		std::string msg = lua_tostring(L, -1);
		if (msg.length() > 0) {
			LogError("LUA ERROR:\n" + msg, line, file);
			luaL_traceback(L, L, msg.c_str(), 1);
			std::string trace = lua_tostring(L, -1);
			LogError(trace, line, file);
		}
	}
}