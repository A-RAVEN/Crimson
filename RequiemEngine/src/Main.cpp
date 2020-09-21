#define NOMINMAX
#include <GPUDevice.h>shaderGroupHandleSize
#include <headers/Win32Window.h>
#include <Compiler.h>
#include <fstream>
#include <array>
#include <assimp/scene.h>
#include <headers/Resources/MeshletResource.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <headers/MeshResource.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <headers/stb_image.h>
#include <headers/VertexData.h>
#include <headers/KeyboardController.h>
#include <headers/GeometryInstanceManager.h>
#include <headers/BufferVector.h>
#include <headers/MeshQueue.h>
#include <headers/TimeManager.h>
#include <headers/ShaderProcessor.h>
#include <headers/ThreadManager.h>
#include <iostream>
#include <headers/Camera.h>
#include <headers/RenderingSystem.h>
#include <headers/LuaInterface/LuaMachine.h>
#include <headers/LuaInterface/LuaInterfaces.h>
#include <headers/NetworkManager.h>
#include <headers/Entity/World.h>
#include <headers/Components/TransformComponent.h>
#include <headers/Components/MeshRenderComp.h>
#include <headers/Components/HierarchyComp.h>
#include <headers/LuaInterface/GraphicsLuaMachine.h>
#include <headers/GlobalStaticPointers.h>

using RaytraceGeometryType = RayTraceGeometryInstance<glm::mat3x4>;


class OneTimeTestJob : public ThreadJob
{
public:
	OneTimeTestJob() {}
	virtual void Work(ThreadWorker const* this_worker) override
	{
		std::cout << "one time job" << " is working on thread " << this_worker->GetId() << std::endl;
	};
	~OneTimeTestJob() {};
};

void TestFunc(int input, std::string value, int input2)
{
	std::cout << "data0 from lua is " << input << " " << input2 << std::endl;
	std::cout << value << std::endl;
}

int TestFunc1(int input, std::string value, int input2)
{
	std::cout << "data1 from lua is " << input << " " << input2 << std::endl;
	std::cout << value << std::endl;
	return 0;
}

class MeshInstance
{
public:
	TransformComponent* transform;
	MeshResource* mesh;
};


int main()
{
	ThreadManager *thread_manager = new ThreadManager();
	thread_manager->Init();
	GLOBAL::GlobalStaticManager = thread_manager;
	NetworkManager network_manager;
	TimeManager time_manager;
	KeyboardController inputs;
	World world(time_manager, inputs);
	EntityId entity = world.AllocateEntity();
	TransformComp* transform = world.AddEntityComponent<TransformComp>(entity);
	HierarchyComp* hierarchy = world.AddEntityComponent<HierarchyComp>(entity);
	MeshRenderComp* meshRenderer = world.AddEntityComponent<MeshRenderComp>(entity);
	world.AddEntity(entity);

	LuaVM lua_machine;
	lua_machine.InitVM();

	std::vector<luaL_Reg> functions =
	{
		{"TestFunc", GET_FUN_WRAPPER(&TestFunc)},
		{"TestFunc1", GET_FUN_WRAPPER(&TestFunc1)}
	};
	lua_machine.RegisterFunctions(functions.data(), functions.size());
	lua_machine.LoadScript("test.lua");



	Camera cam;
	cam.view = glm::lookAt(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	cam.proj = glm::perspective(glm::radians(45.0f), 1024.0f / 720.0f, 0.1f, 1000.0f);
	cam.proj[1][1] *= -1.0f;
	cam.viewInverse = glm::inverse(cam.view);
	cam.projInverse = glm::inverse(cam.proj);


	Assimp::Importer scene_importer;
	const aiScene* scene = scene_importer.ReadFile("sponza.obj", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	std::cout << "Start" << std::endl;
	using namespace Crimson;
	GPUDeviceManager::Init();
	std::cout << "Init Context" << std::endl;
	GPUDeviceManager::Get()->InitAPIContext(EAPIType::E_API_TYPE_VULKAN, true);
	std::cout << "Create Device" << std::endl;
	PGPUDevice MainDevice = GPUDeviceManager::Get()->CreateDevice("MainDevice", 0, EAPIType::E_API_TYPE_VULKAN, 3, 1, 1);

	MeshletGroupResource new_meshlet(MainDevice, MeshletGroupResource::CreateSharedSetLayout(MainDevice));
	new_meshlet.LoadMeshRaw(scene);

	Win32Window new_window;
	new_window.InitWindow(L"Test Window", L"default", 1024, 720);
	MainDevice->RegisterWindow(new_window);

	GraphicsLuaMachine machine;
	machine.InitVM();
	GraphicsContext newContext(MainDevice);
	machine.RegisterGraphicsContext(&newContext, "testgraphics.lua");

	TransformComponentAllocator m_TransformManager;

	MeshResource new_resource;
	new_resource.ProcessAiSceneLightWeight(scene, true, false);



	const aiScene* bunnymesh = scene_importer.ReadFile("bunny.ply", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	MeshResource bunny_resource;
	bunny_resource.ProcessAiSceneLightWeight(bunnymesh, true, false);

	const aiScene* cube = scene_importer.ReadFile("cube.obj", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	MeshResource cube_resource;
	cube_resource.ProcessAiSceneLightWeight(cube, true, false);

	///////////Setup Raytracing Resources
	PRayTraceGeometry raytrace_geometry = MainDevice->CreateRayTraceGeometry();
	raytrace_geometry->SetVertexData(new_resource.m_VertexBuffer, 0, new_resource.m_VertexSize, new_resource.GetVertexStride(), EDataType::EVEC3);
	raytrace_geometry->SetIndexData(new_resource.m_IndexBuffer, 0, new_resource.m_IndexSize, EIndexType::E_INDEX_TYPE_32);
	raytrace_geometry->SetGeometryFlags({ EGeometryFlags::E_GEOMETRY_OPAQUE });
	raytrace_geometry->SetGeometryType(ERayTraceGeometryType::E_GEOMETRY_TYPE_TRIANGLES);

	//Create Blas
	PAccelerationStructure blas = MainDevice->CreateAccelerationStructure();
	blas->m_Geometries = { raytrace_geometry };
	blas->m_BuildFlags = { EBuildAccelerationStructureFlags::E_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV };
	blas->InitAS();
	blas->SetupScratchBuffer();
	PAccelerationStructure tlas = MainDevice->CreateAccelerationStructure();
	tlas->m_InstanceNumber = 10;
	tlas->m_BuildFlags = { EBuildAccelerationStructureFlags::E_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV, EBuildAccelerationStructureFlags::E_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV };
	tlas->InitAS(true);
	tlas->SetupScratchBuffer();
	PGPUBuffer geometry_instance_buffer = MainDevice->CreateBuffer(sizeof(RaytraceGeometryType) * 5, { EBufferUsage::E_BUFFER_USAGE_RAYTRACING_NV }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	RaytraceGeometryType* pGeometryInstance = new (geometry_instance_buffer->GetMappedPointer()) RaytraceGeometryType[5];

	std::vector<MeshInstance> instances;
	std::vector<size_t> volumes;

	BVH test_bvh;

	BufferQueue<uint32_t, 10> transform_queue;
	transform_queue.Init(MainDevice, { EBufferUsage::E_BUFFER_USAGE_STORAGE }, EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	{
		MeshInstance sponza;
		sponza.mesh = &new_resource;
		sponza.transform = m_TransformManager.AllocateTransformComponent();

		pGeometryInstance[0].m_Flags = static_cast<uint32_t>(EGeometryInstanceFlags::E_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE);
		pGeometryInstance[0].m_InstanceId = 0;
		pGeometryInstance[0].m_Mask = 0x1;// 0xff;
		pGeometryInstance[0].m_AccelerationStructureHandle = blas->GetHandle();
		sponza.transform->m_Info.m_Matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		pGeometryInstance[0].m_TransformMatrix = glm::transpose(sponza.transform->m_Info.m_Matrix);
		transform_queue.PushBack(sponza.transform->m_Info.m_TransformId);

		//instances.push_back(sponza);
	}

	for(int i = 0; i < 4; ++i)
	{
		MeshInstance bunny;
		bunny.mesh = &bunny_resource;
		bunny.transform = m_TransformManager.AllocateTransformComponent();
		bunny.transform->m_Info.m_Matrix = glm::translate(glm::mat4(1.0f), glm::vec3(i * 10.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f));
		instances.push_back(bunny);

		size_t volume_id = test_bvh.AllocVolume();
		volumes.push_back(volume_id);
		auto vol = test_bvh[volume_id];
		vol.box = bunny.mesh->GetBox(bunny.transform->m_Info.m_Matrix);
	}
	test_bvh.UpdateTree();
	RenderingSystem rendering_system(&new_window, blas, tlas, geometry_instance_buffer, &new_resource, transform_queue, &new_meshlet, &cube_resource);


	glm::vec2 angles(0.0f);
	glm::vec3 position(-5.0f, 0.0f, 0.0f);

	bool toggle = false;


	bool net_inited = false;
	thread_manager->EnqueueJob(&rendering_system);
	float wait_time = 0.0f;
	while (new_window.IsWindowRunning())
	{
		time_manager.UpdateClock();
		inputs.UpdateController();
		world.Update();

		for (int i = 0; i < 4; ++i)
		{
			MeshInstance &bunny = instances[i];
			bunny.transform->m_Info.m_Matrix = glm::translate(glm::mat4(1.0f), glm::vec3(i * 10.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f));
			auto& vol = test_bvh[volumes[i]];
			vol.box = bunny.mesh->GetBox(bunny.transform->m_Info.m_Matrix);
			test_bvh.VolumeDirty(volumes[i]);
		}
		//test_bvh.UpdateTree();


		//transforms[0]->m_Info.m_Matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, glm::sin(time_manager.elapsedTime()), 0.0f)) * glm::rotate(glm::mat4(1.0f), time_manager.elapsedTime() * glm::pi<float>() * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));

		if (inputs.KeyTriggered(inputs.GetCharKey('C')))
		{
			toggle = !toggle;
		}

		if (inputs.KeyState(VK_RBUTTON))
		{
			glm::vec2 movement = inputs.GetMouseMovement();
			angles.x = (angles.x + movement.x * time_manager.deltaTime() * 1.0f);
			angles.y = glm::clamp(angles.y + movement.y * time_manager.deltaTime() * 1.0f, -glm::pi<float>() * 0.49999f, glm::pi<float>() * 0.49999f);
		}
		glm::vec3 forward = glm::rotate(glm::mat4(1.0f), -angles.x, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), -angles.y, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
		if (new_window.Focused())
		{
			if (inputs.KeyState(inputs.GetCharKey('W')))
			{
				position += forward * 15.0f * time_manager.deltaTime();
			}
			if (inputs.KeyState(inputs.GetCharKey('S')))
			{
				position -= forward * 15.0f * time_manager.deltaTime();
			}
			if (inputs.KeyState(inputs.GetCharKey('D')))
			{
				position += right * 15.0f * time_manager.deltaTime();
			}
			if (inputs.KeyState(inputs.GetCharKey('A')))
			{
				position -= right * 15.0f * time_manager.deltaTime();
			}
			if (inputs.KeyTriggered(inputs.GetCharKey('C')) && !net_inited)
			{
				network_manager.CreateAgent();
				thread_manager->EnqueueJob(&network_manager);
				net_inited = true;
			}
			else if (inputs.KeyTriggered(inputs.GetCharKey('V')) && !net_inited)
			{
				network_manager.CreateAgent(true, 4);
				thread_manager->EnqueueJob(&network_manager);
				net_inited = true;
			}
		}
		{
			cam.view = glm::lookAt(position, position + forward, glm::vec3(0.0f, 1.0f, 0.0f));
			cam.viewInverse = glm::inverse(cam.view);
		}
		GraphicsFrame new_frame{};
		m_TransformManager.GenerateGraphicsFrame(new_frame);
		for (auto& trans : instances)
		{
			InstanceInfo new_info{};
			new_info.m_BatchId = trans.transform->m_Info.m_BatchId;
			new_info.m_TransformId = trans.transform->m_Info.m_TransformId;
			new_info.p_Mesh = trans.mesh;
			new_frame.AddInstance(new_info);
		}
		new_frame.m_Camera = cam;
		auto& volumes = test_bvh.GetActiveVolumes();
		for (auto& vol : volumes)
		{
			new_frame.debugVolumes.push_back(test_bvh[vol]);
		}
		
		rendering_system.PushBackNewFrame(new_frame);

		new_window.UpdateWindow();

		wait_time = rendering_system.DeltaTimeApprox() - time_manager.deltaTime();
	}
	thread_manager->Terminate();
	delete thread_manager;
	return 0;
}