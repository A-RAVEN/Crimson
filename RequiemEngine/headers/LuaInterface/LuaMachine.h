#pragma once
#include <string>

struct lua_State;
struct luaL_Reg;
class LuaVM
{
public:
	void InitVM();
	void LoadScript(std::string const& _script_name);
	void RegisterFunctions(luaL_Reg const* funcs, size_t reg_count);
	void RegisterModules(luaL_Reg const* funcs, size_t reg_count);
	void RegisterLuaClass(std::string const& className);
	void ReleaseVM();
	lua_State* GetState() { return m_VM; }
private:
	lua_State* m_VM;
};