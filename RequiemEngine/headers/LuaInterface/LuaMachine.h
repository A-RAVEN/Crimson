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
	//new
	//void AddEntityLuaScript(int32_t entity_ID, std::string const& script_name, std::map<std::string, std::string>const& script_table);
	//void RemoveEntityLuaScripts(int32_t entity_ID);
	//void UpdateLuaScripts(float delta_time);

	//void AppendPath(std::string const& _path);
	void ReleaseVM();
	lua_State* GetState() { return m_VM; }
private:
	lua_State* m_VM;
};