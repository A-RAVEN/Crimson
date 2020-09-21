#pragma once
#include <vector>
#include <Generals.h>
#include <GPUDevice.h>
#include <glm/glm.hpp>
#include <deque>
#include <array>
#include <headers/GraphicsFrame.h>
#include <unordered_set>

using namespace Crimson;

struct TransformData
{
	glm::mat4 m_ModelTransform;
};

class TransformComponent
{
public:
	TransformComponent() :
		m_Info(glm::mat4(1.0f), 0, 0)
	{}
	TransformInfo m_Info;
};

class TransformComponentAllocator
{
public:
	TransformComponent* AllocateTransformComponent();
	void RecycleTransformComponent(TransformComponent* transform_comp);
	struct TransformIdPage
	{
		TransformIdPage();
		std::deque<uint32_t> m_AvailableIds;
		uint32_t m_LastIndex;
		uint32_t m_MaxNum;
		bool isFull() const;
		uint32_t AllocateTransformId();
		void ReturnTransformIs(uint32_t id);
	};
	void GenerateGraphicsFrame(GraphicsFrame &frame);
private:
	std::deque<TransformIdPage> m_MegaTransformPages;
	std::deque<TransformComponent*> m_RecycledTransformComponent;
	std::deque<TransformComponent> m_TransformComponentPool;
	std::unordered_set<TransformComponent*> m_UsingSet;
};

class TransformManager
{
public:
	TransformManager();
	void ExtendBufferPages(uint32_t batch_id);
	PDescriptorSetLayout GetSetLayout() const { return m_TransformSetLayout; }
	PDescriptorSet GetSet(uint32_t batch_id) const { return m_TransformSets[batch_id]; }
	uint32_t GetBatchCount() const { return m_MegaTransformBatchsBuffers.size(); }
	TransformData* GetData(uint32_t batch_id, uint32_t transform_id);

	struct TransformBufferData
	{
		PGPUBuffer m_Buffer;
		TransformData* p_Data;
		uint32_t m_LastIndex;
		uint32_t m_MaxNum;
		std::deque<uint32_t> m_AvailableTransforms;
		TransformBufferData();
		void Init(uint32_t number, PGPUDevice device);
		TransformBufferData(const TransformBufferData& other);
		bool isFull() const;
		TransformData* AllocateData(uint32_t &offset);
		void ReleaseData(uint32_t offset);
	};
private:
	PGPUDevice m_Device;
	std::deque<TransformBufferData> m_MegaTransformBatchsBuffers;
	PDescriptorSetLayout m_TransformSetLayout;
	std::vector<PDescriptorSet> m_TransformSets;
};