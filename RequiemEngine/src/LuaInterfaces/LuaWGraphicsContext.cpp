#include <headers/LuaInterfaces/LuaWGraphicsContext.h>

namespace
{


	const luaL_Reg lua_reg_graphics_funcs[] =
	{
		{"CreateResizableRenderTarget",GET_MEM_FUN_WRAPPER(&GraphicsContext::CreateResizableRenderTarget)},
		{"CreateRenderPassInstance",GET_MEM_FUN_WRAPPER(&GraphicsContext::CreateRenderPassInstance)},
		{"CreateFramebuffer",GET_MEM_FUN_WRAPPER(&GraphicsContext::CreateFramebuffer)},
		{NULL,NULL},
	};
}

void LuaWGraphicsContext::InjectMetaTable(lua_State* L, std::string const& name)
{
	lua_newtable(L);
	luaL_newlib(L, lua_reg_graphics_funcs);
	lua_setfield(L, -2, "__index");
	lua_setfield(L, LUA_REGISTRYINDEX, name.c_str());
}
