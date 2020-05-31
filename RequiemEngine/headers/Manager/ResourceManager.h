#pragma once
#include <headers/MeshResource.h>
#include <headers/Resources/ShaderGroup.h>
#include <deque>
#include <headers/ShaderProcessor.h>
#include <map>
#include <headers/ThreadManager.h>

//template<typename T>
//class ResourcePool
//{
//private:
//	std::deque<T> resource;
//};
template <typename T>
class ResourceAllocator
{
	static_assert(std::is_base_of<IResource, T>::value, "type T should be derived from class IComponent!");
public:
	T* GetResource(std::string const& key)
	{
		auto find = m_ResourceNameMap.find(key);
		if (find != m_ResourceNameMap.end())
		{
			return &m_ResourcePool[find->second];
		}
		return nullptr;
	}
	T* AllocResource(std::string const& key)
	{
		T* result = GetResource(key);
		if (result == nullptr)
		{
			size_t top = m_ResourcePool.size();
			if (!m_AvailableIndices.empty())
			{
				top = m_AvailableIndices.front();
				m_AvailableIndices.pop_front();
			}
			else
			{
				m_ResourcePool.push_back(T{});
			}
			m_ResourceNameMap.insert(std::make_pair(key, top));
			
			result = &m_ResourcePool[top];
		}
		return result;
	}
	void FreeResource(std::string const& key)
	{
		auto find = m_ResourceNameMap.find(key);
		if (find != m_ResourceNameMap.end())
		{
			m_AvailableIndices.push_back(find->second);
			m_ResourceNameMap.erase(find);
		}
	}
	void FreeResource(T* resource)
	{
		FreeResource(T->GetName());
	}
private:
	//shader module resources
	std::deque<size_t> m_AvailableIndices;
	std::map<std::string, size_t> m_ResourceNameMap;
	std::deque<T> m_ResourcePool;
};

class ShaderModuleResourceManager
{
public:
	void Init(PGPUDevice device);
	void LoadShaderSource(std::string const& shader_name);
	PipelinePackageResource* GetPackage(std::string const& package_name);
	void Free();
private:
	PGPUDevice m_Device;
	ShaderProcessor m_ShaderProcessor;
	ResourceAllocator<PipelinePackageResource> m_Allocator;
};

class ResourceManager
{
public:
	void InitResourceManager(PGPUDevice device);
	void LoadShaderModule(std::string shader_name);
	MeshResource* GetMeshResource(std::string const& mesh_name);
private:
	ThreadManager*	p_ThreadManager;
	PGPUDevice		m_Device;

	ShaderModuleResourceManager m_ShaderModuleResourceManager;
};