#pragma once
#include <headers/LuaInterface/LuaInterfaces.h>
#include <headers/Graphics/GraphicsRenderPipeline.h>

struct lua_State;
class LuaWGraphicsContext
{
public:
	static void InjectMetaTable(lua_State* L, std::string const& name);
};
