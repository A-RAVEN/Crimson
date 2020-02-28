#include <headers/GeometryInstanceManager.h>

TransformManager::TransformBufferData::TransformBufferData() :
	m_Buffer(nullptr),
	p_Data(nullptr),
	m_LastIndex(0),
	m_MaxNum(0)
{}

void TransformManager::TransformBufferData::Init(uint32_t number, PGPUDevice device)
{
	m_MaxNum = number;
	m_Buffer = device->CreateBuffer(sizeof(TransformData) * m_MaxNum,
		{ EBufferUsage::E_BUFFER_USAGE_COPY_DST, EBufferUsage::E_BUFFER_USAGE_STORAGE },
		EMemoryType::E_MEMORY_TYPE_HOST_TO_DEVICE);
	p_Data = new (m_Buffer->GetMappedPointer()) TransformData();
}

TransformManager::TransformBufferData::TransformBufferData(const TransformBufferData& other) :
	m_Buffer(other.m_Buffer),
	p_Data(other.p_Data),
	m_LastIndex(other.m_LastIndex),
	m_MaxNum(other.m_MaxNum),
	m_AvailableTransforms(other.m_AvailableTransforms)
{
}

bool TransformManager::TransformBufferData::isFull() const
{
	return m_AvailableTransforms.empty() && m_LastIndex >= m_MaxNum;
}

TransformData* TransformManager::TransformBufferData::AllocateData(uint32_t& offset)
{
	if (!m_AvailableTransforms.empty())
	{
		offset = m_AvailableTransforms.front();
		m_AvailableTransforms.pop_front();
		return &p_Data[offset];
	}
	if (m_LastIndex < m_MaxNum)
	{
		offset = m_LastIndex++;
		return &p_Data[offset];
	}
	return nullptr;
}

void TransformManager::TransformBufferData::ReleaseData(uint32_t offset)
{
	m_AvailableTransforms.push_back(offset);
}

TransformManager::TransformManager()
{
	m_Device = GPUDeviceManager::Get()->GetDevice("MainDevice");
	m_TransformSetLayout = m_Device->CreateDescriptorSetLayout();
	m_TransformSetLayout->m_Bindings.resize(1);
	m_TransformSetLayout->m_Bindings[0].m_Num = 1;
	m_TransformSetLayout->m_Bindings[0].m_BindingPoint = 0;
	m_TransformSetLayout->m_Bindings[0].m_ResourceType = EShaderResourceType::E_SHADER_TYPE_STORAGE_BUFFER;
	m_TransformSetLayout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_VERTEX };
}

TransformComponent* TransformManager::AllocateTransformComponent()
{
	TransformComponent* return_val = new TransformComponent();
	for(uint32_t batch_id = 0; batch_id < m_MegaTransformBatchsBuffers.size(); ++batch_id)
	{
		if (!m_MegaTransformBatchsBuffers[batch_id].isFull())
		{
			return_val->m_BatchId = batch_id;
			return_val->m_RawPointer = m_MegaTransformBatchsBuffers[batch_id].AllocateData(return_val->m_Offset);
			return return_val;
		}
	}
	m_MegaTransformBatchsBuffers.push_back(TransformBufferData{});
	return_val->m_BatchId = m_MegaTransformBatchsBuffers.size() - 1;
	m_MegaTransformBatchsBuffers[return_val->m_BatchId].Init(1024, m_Device);
	PDescriptorSet new_set = m_TransformSetLayout->AllocDescriptorSet();
	new_set->WriteDescriptorSetBuffers(0, { m_MegaTransformBatchsBuffers[return_val->m_BatchId].m_Buffer }, { BufferRange{0, m_MegaTransformBatchsBuffers[return_val->m_BatchId].m_Buffer->GetSize()} }, 0);
	new_set->EndWriteDescriptorSet();
	m_TransformSets.push_back(new_set);
	return_val->m_RawPointer = m_MegaTransformBatchsBuffers[return_val->m_BatchId].AllocateData(return_val->m_Offset);
	return return_val;
}
