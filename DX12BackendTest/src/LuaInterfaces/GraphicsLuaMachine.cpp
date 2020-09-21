#include <headers/LuaInterface/LuaInterfaces.h>
#include <headers/LuaInterface/GraphicsLuaMachine.h>
#include <headers/LuaInterfaces/LuaWGraphicsContext.h>
#include <headers/HelperFunc.h>

constexpr char GRAPHICS_CONTEXT_FUNC_TABLE_NAME[] = "BUILTIN_GRAPHICS_CONTEXT";
void GraphicsLuaMachine::InitVM()
{
	m_VM = luaL_newstate();
	luaL_openlibs(m_VM);
	lua_newtable(m_VM);
	lua_setglobal(m_VM, "GLOBAL_SCRIPT_TABLE");
	lua_settop(m_VM, 0);
	LuaWGraphicsContext::InjectMetaTable(m_VM, GRAPHICS_CONTEXT_FUNC_TABLE_NAME);
	RegisterEnums();
}

void GraphicsLuaMachine::RegisterGraphicsContext(GraphicsContext* context, std::string const& context_script)
{
	std::string script = LoadStringFile(context_script);
	CHECK_LUAERR(m_VM, luaL_loadstring(m_VM, script.c_str()));
	std::string uniq_name = std::to_string((uintptr_t)context);

	lua_newtable(m_VM);
	lua_setfield(m_VM, LUA_REGISTRYINDEX, uniq_name.c_str());
	lua_getfield(m_VM, LUA_REGISTRYINDEX, uniq_name.c_str());

	//add graphics context for upvalue table
	WrapPUserData(m_VM, context);
	lua_getfield(m_VM, LUA_REGISTRYINDEX, GRAPHICS_CONTEXT_FUNC_TABLE_NAME);
	lua_setmetatable(m_VM, -2);
	lua_setfield(m_VM, -2, "context");

	//add global as metatable for upvalue table
	lua_newtable(m_VM);
	lua_getglobal(m_VM, "_G");
	lua_setfield(m_VM, -2, "__index");
	lua_setmetatable(m_VM, -2);

	lua_setupvalue(m_VM, -2, 1);

	CHECK_LUAERR(m_VM, lua_pcall(m_VM, 0, 0, 0));
	lua_settop(m_VM, 0);
}

void GraphicsLuaMachine::RegisterEnums()
{
	std::vector<std::pair<std::string, uint32_t>> enum_table =
	{
		MAKE_ENUM_PAIR(EFormat::E_FORMAT_R8_UNORM),
		MAKE_ENUM_PAIR(EFormat::E_FORMAT_R8G8_UNORM),

		MAKE_ENUM_PAIR(EFormat::E_FORMAT_R8G8B8_UNORM),

		MAKE_ENUM_PAIR(EFormat::E_FORMAT_B8G8R8_UNORM),

		MAKE_ENUM_PAIR(EFormat::E_FORMAT_R8G8B8A8_UNORM),

		MAKE_ENUM_PAIR(EFormat::E_FORMAT_B8G8R8A8_UNORM),

		MAKE_ENUM_PAIR(EFormat::E_FORMAT_R16_UNORM),

		MAKE_ENUM_PAIR(EFormat::E_FORMAT_R32_SFLOAT),

		MAKE_ENUM_PAIR(EFormat::E_FORMAT_R32G32B32A32_SFLOAT),

		MAKE_ENUM_PAIR(EFormat::E_FORMAT_D16_UNORM),

		MAKE_ENUM_PAIR(EFormat::E_FORMAT_D32_SFLOAT),
	};

	lua_newtable(m_VM);
	for (auto& enum_pair : enum_table)
	{
		lua_pushinteger(m_VM, enum_pair.second);
		lua_setfield(m_VM, -2, enum_pair.first.c_str());
	}
	lua_setglobal(m_VM, "EFormat");
}
