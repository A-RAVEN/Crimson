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
	m_TransformSetLayout->m_Bindings[0].m_ShaderTypes = { EShaderType::E_SHADER_TYPE_VERTEX, EShaderType::E_SHADER_TYPE_CLOSEHIT_NV };
	m_TransformSetLayout->BuildLayout();
}

void TransformManager::ExtendBufferPages(uint32_t batch_id)
{
	if (batch_id >= m_MegaTransformBatchsBuffers.size())
	{
		for (int i = m_MegaTransformBatchsBuffers.size(); i <= batch_id; ++i)
		{
			m_MegaTransformBatchsBuffers.push_back(TransformBufferData{});

			m_MegaTransformBatchsBuffers.back().Init(500, m_Device);

			PDescriptorSet new_set = m_TransformSetLayout->AllocDescriptorSet();
			new_set->WriteDescriptorSetBuffers(0, { m_MegaTransformBatchsBuffers.back().m_Buffer }, { BufferRange{0, m_MegaTransformBatchsBuffers.back().m_Buffer->GetSize()} }, 0);
			new_set->EndWriteDescriptorSet();
			m_TransformSets.push_back(new_set);
		}
	}
}

TransformData* TransformManager::GetData(uint32_t batch_id, uint32_t transform_id)
{
	return &m_MegaTransformBatchsBuffers[batch_id].p_Data[transform_id];
}

TransformComponent* TransformComponentAllocator::AllocateTransformComponent()
{
	if (!m_RecycledTransformComponent.empty())
	{
		TransformComponent* return_val = m_RecycledTransformComponent.front();
		m_RecycledTransformComponent.pop_front();
		m_UsingSet.insert(return_val);
		return return_val;
	}
	m_TransformComponentPool.push_back(TransformComponent{});
	TransformComponent* return_val = &(m_TransformComponentPool.back());

	for(uint32_t batch_id = 0; batch_id < m_MegaTransformPages.size(); ++batch_id)
	{
		if (!m_MegaTransformPages[batch_id].isFull())
		{
			return_val->m_Info.m_BatchId = batch_id;
			return_val->m_Info.m_TransformId = m_MegaTransformPages[batch_id].AllocateTransformId();
			m_UsingSet.insert(return_val);
			return return_val;
		}
	}
	m_MegaTransformPages.push_back(TransformIdPage{});
	return_val->m_Info.m_BatchId = m_MegaTransformPages.size() - 1;
	//m_MegaTransformPages[return_val->m_Info.m_BatchId].Init(1024, m_Device);
	//PDescriptorSet new_set = m_TransformSetLayout->AllocDescriptorSet();
	//new_set->WriteDescriptorSetBuffers(0, { m_MegaTransformBatchsBuffers[return_val->m_BatchId].m_Buffer }, { BufferRange{0, m_MegaTransformBatchsBuffers[return_val->m_BatchId].m_Buffer->GetSize()} }, 0);
	//wwwwwwwwwnew_set->EndWriteDescriptorSet();
	//m_TransformSets.push_back(new_set);
	return_val->m_Info.m_TransformId = m_MegaTransformPages[return_val->m_Info.m_BatchId].AllocateTransformId();
	//return_val->m_RawPointer = m_MegaTransformBatchsBuffers[return_val->m_BatchId].AllocateData(return_val->m_Offset);
	m_UsingSet.insert(return_val);
	return return_val;
}

void TransformComponentAllocator::RecycleTransformComponent(TransformComponent* transform_comp)
{
	if (m_UsingSet.erase(transform_comp))
	{
		m_RecycledTransformComponent.push_back(transform_comp);
	}
}

void TransformComponentAllocator::GenerateGraphicsFrame(GraphicsFrame& frame)
{
	for (TransformComponent* component : m_UsingSet)
	{
		frame.m_TransoformUpdateInfo.push_back(component->m_Info);
	}
}

TransformComponentAllocator::TransformIdPage::TransformIdPage() : 
	m_MaxNum(500),
	m_LastIndex(0)
{
}

bool TransformComponentAllocator::TransformIdPage::isFull() const
{
	return m_AvailableIds.empty() && m_LastIndex == m_MaxNum;
}

uint32_t TransformComponentAllocator::TransformIdPage::AllocateTransformId()
{
	if (!m_AvailableIds.empty())
	{
		uint32_t front = m_AvailableIds.front();
		m_AvailableIds.pop_front();
		return front;
	}
	if (m_LastIndex < m_MaxNum)
	{
		return m_LastIndex++;
	}
	return std::numeric_limits<uint32_t>::max();
}

void TransformComponentAllocator::TransformIdPage::ReturnTransformIs(uint32_t id)
{
	if (m_LastIndex == (id + 1))
	{
		--m_LastIndex;
		return;
	}
	m_AvailableIds.push_back(id);
}
