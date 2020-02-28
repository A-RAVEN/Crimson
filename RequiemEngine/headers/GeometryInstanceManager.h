#pragma once
#include <vector>
#include <Generals.h>
#include <GPUDevice.h>
#include <glm/glm.hpp>
#include <deque>
#include <array>

using namespace Crimson;

struct TransformData
{
	glm::mat4 m_ModelTransform;
};

class TransformComponent
{
public:
	TransformComponent() :
		m_BatchId(0),
		m_Offset(0),
		m_RawPointer(nullptr)
	{}
	uint32_t m_BatchId;
	uint32_t m_Offset;
	TransformData* m_RawPointer;
};

class TransformManager
{
public:
	TransformManager();
	TransformComponent* AllocateTransformComponent();
	PDescriptorSetLayout GetSetLayout() const { return m_TransformSetLayout; }
	PDescriptorSet GetSet(uint32_t batch_id) const { return m_TransformSets[batch_id]; }
	uint32_t GetBatchCount() const { return m_MegaTransformBatchsBuffers.size(); }

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