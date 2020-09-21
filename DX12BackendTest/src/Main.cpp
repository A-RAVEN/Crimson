#define NOMINMAX
#include <GPUDevice.h>shaderGroupHandleSize
#include <headers/Win32Window.h>
#include <Compiler.h>
#include <fstream>
#include <array>
#include <assimp/scene.h>
//#include <headers/Resources/MeshletResource.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <headers/MeshResource.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <headers/stb_image.h>
#include <headers/VertexData.h>
#include <headers/KeyboardController.h>
//#include <headers/GeometryInstanceManager.h>
//#include <headers/BufferVector.h>
//#include <headers/MeshQueue.h>
#include <headers/TimeManager.h>
#include <headers/ShaderProcessor.h>
//#include <headers/ThreadManager.h>
#include <iostream>
#include <headers/Camera.h>
//#include <headers/RenderingSystem.h>
//#include <headers/LuaInterface/LuaMachine.h>
//#include <headers/LuaInterface/LuaInterfaces.h>
//#include <headers/NetworkManager.h>
//#include <headers/Entity/World.h>
//#include <headers/Components/TransformComponent.h>
//#include <headers/Components/MeshRenderComp.h>
//#include <headers/Components/HierarchyComp.h>
//#include <headers/LuaInterface/GraphicsLuaMachine.h>

int main()
{
	using namespace Crimson;
	GPUDeviceManager::Init();
	std::cout << "Init Context" << std::endl;
	GPUDeviceManager::Get()->InitAPIContext(EAPIType::E_API_TYPE_D3D12, true);
	std::cout << "Create Device" << std::endl;
	PGPUDevice MainDevice = GPUDeviceManager::Get()->CreateDevice("MainDevice", 0, EAPIType::E_API_TYPE_D3D12, 3, 1, 1);

	Win32Window new_window;
	new_window.InitWindow(L"Test Window", L"default", 1024, 720);
	MainDevice->RegisterWindow(new_window);
	while (new_window.IsWindowRunning())
	{
		new_window.UpdateWindow();
	}
	GPUDeviceManager::Get()->Dispose();
}