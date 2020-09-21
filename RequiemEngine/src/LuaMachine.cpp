#include <headers/LuaInterface/LuaInterfaces.h>
#include <headers/LuaInterface/LuaMachine.h>
#include <headers/HelperFunc.h>

void LuaVM::InitVM()
{
	m_VM = luaL_newstate();
	luaL_openlibs(m_VM);
	lua_newtable(m_VM);
	lua_setglobal(m_VM, "GLOBAL_SYSTEM_TABLE");
	lua_newtable(m_VM);
	lua_setglobal(m_VM, "GLOBAL_SCRIPT_TABLE");
	lua_settop(m_VM, 0);
}

void LuaVM::LoadScript(std::string const& _script_name)
{
	std::string script = LoadStringFile(_script_name);
	CHECK_LUAERR(m_VM, luaL_loadstring(m_VM, script.c_str()));

	//environment table, i.e. container table of script
	lua_newtable(m_VM);
	lua_setfield(m_VM, LUA_REGISTRYINDEX, _script_name.c_str());
	lua_getfield(m_VM, LUA_REGISTRYINDEX, _script_name.c_str());
	//meta table
	lua_newtable(m_VM);
	lua_getglobal(m_VM, "_G");
	lua_setfield(m_VM, -2, "__index");
	lua_setmetatable(m_VM, -2);
	//upvalue of script,
	lua_setupvalue(m_VM, -2, 1);
	//execute script
	CHECK_LUAERR(m_VM, lua_pcall(m_VM, 0, 0, 0));
	lua_settop(m_VM, 0);
}

void LuaVM::RegisterFunctions(luaL_Reg const* funcs, size_t reg_count)
{
	for (size_t i = 0; i < reg_count; ++i)
	{
		lua_pushcfunction(m_VM, funcs[i].func);
		lua_setglobal(m_VM, funcs[i].name);
	}
	lua_settop(m_VM, 0);
}

void LuaVM::RegisterModules(luaL_Reg const* funcs, size_t reg_count)
{
	for (int i = 0; i < reg_count; ++i)
	{
		luaL_requiref(m_VM, funcs[i].name, funcs[i].func, 0);
		lua_pop(m_VM, 1);
	}
	lua_settop(m_VM, 0);
}

void LuaVM::RegisterLuaClass(std::string const& className)
{

}

void LuaVM::ReleaseVM()
{
	lua_settop(m_VM, 0);
	lua_close(m_VM);
}
