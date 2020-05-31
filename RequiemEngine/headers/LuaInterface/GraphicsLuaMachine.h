#pragma once
#include <headers/Graphics/GraphicsRenderPipeline.h>

struct lua_State;
struct luaL_Reg;
class GraphicsLuaMachine
{
public:
	void InitVM();
	void RegisterGraphicsContext(GraphicsContext* context, std::string const& context_script);
	//void AddFunctionTable(std::string const& name, )
private:
	void RegisterEnums();
	lua_State* m_VM;
};