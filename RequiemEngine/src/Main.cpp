#define NOMINMAX
#include <GPUDevice.h>shaderGroupHandleSize
#include <headers/Win32Window.h>
#include <Compiler.h>
#include <fstream>
#include <array>
#include <assimp/scene.h>
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


class TestJob : public ThreadJob
{
public:
	TestJob() : m_JobId(0) {}
	virtual void Work(ThreadWorker const* this_worker) override
	{
		while (this_worker->Working())
		{
			//std::cout << "job " << m_JobId << " is working on thread " << this_worker->GetId() << std::endl;
		}
		std::cout << "terminate" << std::endl;
	};
	~TestJob() {};
	uint32_t m_JobId;
};

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


int main()
{
	Camera cam;
	cam.view = glm::lookAt(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	cam.proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	cam.proj[1][1] *= -1.0f;
	cam.viewInverse = glm::inverse(cam.view);
	cam.projInverse = glm::inverse(cam.proj);


	Assimp::Importer scene_importer;
	const aiScene* scene = scene_importer.ReadFile("testslime.obj", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	std::cout << "Start" << std::endl;
	using namespace Crimson;
	GPUDeviceManager::Init();
	std::cout << "Init Context" << std::endl;
	GPUDeviceManager::Get()->InitAPIContext(EAPIType::E_API_TYPE_VULKAN, true);
	std::cout << "Create Device" << std::endl;
	PGPUDevice MainDevice = GPUDeviceManager::Get()->CreateDevice("MainDevice", 0, EAPIType::E_API_TYPE_VULKAN, 3, 1, 1);


	Win32Window new_window;
	new_window.InitWindow(L"Test Window", L"default", 1024, 720);
	MainDevice->RegisterWindow(new_window);

	RenderingSystem rendering_system(&new_window);

	TransformComponentAllocator m_TransformManager;

	MeshResource new_resource;
	new_resource.ProcessAiScene(scene);
	//MeshInstanceQueue new_queue;
	//new_queue.m_Resource = &new_resource;

	std::vector<TransformComponent*> transforms;
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			TransformComponent* p_component = m_TransformManager.AllocateTransformComponent();
			p_component->m_Info.m_Matrix = glm::translate(glm::mat4(1.0f), glm::vec3(i * 2.0f, 0.0f, j * 2.0f));
			//new_queue.PushInstance(p_component);
			transforms.push_back(p_component);
		}
	}

	

	KeyboardController inputs;

	glm::vec2 angles(0.0f);
	glm::vec3 position(-5.0f, 0.0f, 0.0f);

	bool toggle = false;
	TimeManager time_manager;

	ThreadManager thread_manager;
	thread_manager.Init();
	thread_manager.EnqueueJob(&rendering_system);
	//std::vector<TestJob> new_jobs(5);
	//uint32_t job_id = 0;
	//for (auto& job : new_jobs)
	//{
	//	job.m_JobId = job_id++;
	//	thread_manager.EnqueueJob(&job);
	//}
	while (new_window.IsWindowRunning())
	{
		time_manager.UpdateClock();
		inputs.UpdateController();

		//std::vector<OneTimeTestJob> one_time_jobs(10);
		//for (auto& job : one_time_jobs)
		//{
		//	thread_manager.EnqueueJob(&job);
		//}
		//for (auto& job : one_time_jobs)
		//{
		//	job.WaitJob();
		//}

		transforms[0]->m_Info.m_Matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, glm::sin(time_manager.elapsedTime()), 0.0f)) * glm::rotate(glm::mat4(1.0f), time_manager.elapsedTime() * glm::pi<float>() * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));

		if (inputs.KeyTriggered(inputs.GetCharKey('C')))
		{
			toggle = !toggle;
		}
		//if (inputs.KeyTriggered(inputs.GetCharKey('V')))
		//{
		//	if (pGeometryInstance[1].m_Mask != 0)
		//	{
		//		pGeometryInstance[1].m_Mask = 0;
		//	}
		//	else
		//	{
		//		pGeometryInstance[1].m_Mask = 1;
		//	}
		//}

		if (inputs.KeyState(VK_RBUTTON))
		{
			glm::vec2 movement = inputs.GetMouseMovement();
			angles.x = (angles.x + movement.x * 0.01f);
			angles.y = glm::clamp(angles.y + movement.y * 0.01f, -glm::pi<float>() * 0.49999f, glm::pi<float>() * 0.49999f);
		}
		glm::vec3 forward = glm::rotate(glm::mat4(1.0f), -angles.x, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), -angles.y, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
		if (inputs.KeyState(inputs.GetCharKey('W')))
		{
			position += forward * 0.005f;
		}
		if (inputs.KeyState(inputs.GetCharKey('S')))
		{
			position -= forward * 0.005f;
		}
		if (inputs.KeyState(inputs.GetCharKey('D')))
		{
			position += right * 0.005f;
		}
		if (inputs.KeyState(inputs.GetCharKey('A')))
		{
			position -= right * 0.005f;
		}
		{
			cam.view = glm::lookAt(position, position + forward, glm::vec3(0.0f, 1.0f, 0.0f));
			cam.viewInverse = glm::inverse(cam.view);
			//memcpy(camera_buffer->GetMappedPointer(), &cam, sizeof(Camera));
		}
		GraphicsFrame new_frame{};
		m_TransformManager.GenerateGraphicsFrame(new_frame);
		for (auto& trans : transforms)
		{
			InstanceInfo new_info{};
			new_info.m_BatchId = trans->m_Info.m_BatchId;
			new_info.m_TransformId = trans->m_Info.m_TransformId;
			new_info.p_Mesh = &new_resource;
			new_frame.AddInstance(new_info);
		}
		new_frame.m_Camera = cam;
		rendering_system.PushBackNewFrame(new_frame);

		//*(reinterpret_cast<glm::mat3x4*>(&pGeometryInstance->m_TransformMatrix)) = glm::transpose(transforms[0]->m_Info.m_Matrix);



		new_window.UpdateWindow();
	}
	thread_manager.Terminate();
	return 0;
}