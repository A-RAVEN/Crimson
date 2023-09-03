#include "private/include/pch.h"
#include "ShaderBindingSet_Impl.h"

//align, size
std::pair<size_t, size_t> GetVectorAlignment(uint32_t vector_size)
{
	switch (vector_size)
	{
	case 1:
		return { 1, 1 };
	case 2:
		return { 2, 2 };
	case 3:
		return { 4, 3 };
	case 4:
		return { 4, 4 };
	default:
		CA_LOG_ERR(std::string("UnSupported Vector Size: %d", vector_size));
	}
	return { 0, 0 };
}

//Column Major
std::pair<size_t, size_t> GetMatrixAlignment(uint32_t column, uint32_t row)
{
	return { 4, 4 * row };
}

graphics_backend::ShaderBindingSet_Impl::ShaderBindingSet_Impl(CVulkanApplication& owner) : 
	BaseApplicationSubobject(owner)
{
}

void graphics_backend::ShaderBindingSet_Impl::SetArithmeticValue(std::string const& name, void* pValue)
{
	auto& positions = p_Metadata->GetArithmeticValuePositions();
	auto found = positions.find(name);
	if (found == positions.end())
	{
		CA_LOG_ERR("shader variable " + name + " not found in " + p_Metadata->GetSetSpaceName());
		return;
	}
	size_t currentSizeInbytes = m_UploadData.size();
	size_t requiredSizeInBytes = sizeof(uint32_t) * (found->second.first + found->second.second);
	if (requiredSizeInBytes > currentSizeInbytes)
	{
		m_UploadData.resize(requiredSizeInBytes);
	}
	memcpy(&m_UploadData[sizeof(uint32_t) * found->second.first]
		, pValue
		, sizeof(uint32_t) * found->second.second);
}

void graphics_backend::ShaderBindingSet_Impl::Initialize(ShaderBindingSetMetadata const* inMetaData)
{
	p_Metadata = inMetaData;
}

void graphics_backend::ShaderBindingSet_Impl::DoUpload()
{

}

size_t AccumulateOnOffset(size_t& inoutOffset, std::pair<size_t, size_t> const& inAlign_Size)
{
	size_t next_offset = (inoutOffset + inAlign_Size.first - 1) / inAlign_Size.first * inAlign_Size.first;
	inoutOffset = next_offset + inAlign_Size.second;
	return next_offset;
}

void graphics_backend::ShaderBindingSetMetadata::Initialize(ShaderBindingBuilder const& builder)
{
	p_Builder = &builder;
	auto& descriptors = builder.GetNumericDescriptors();
	m_ArithmeticValuePositions.clear();
	m_TotalSize = 0;
	for (auto& descPairs : descriptors)
	{
		auto& desc = descPairs.second;
		auto& descName = descPairs.first;
		uint32_t vectorSize = desc.x * desc.y;
		uint32_t count = desc.count;
		CA_ASSERT(count > 0
			, "descriptor must have at least one element");
		CA_ASSERT(m_ArithmeticValuePositions.find(descName) == m_ArithmeticValuePositions.end()
			, "descriptor must have at least one element");
		std::pair<size_t, size_t>align_size;
		if (count == 1)
		{
			align_size = GetVectorAlignment(vectorSize);
		}
		else
		{
			align_size = std::make_pair(4, 4 * count);
		}
		size_t descOffset = AccumulateOnOffset(m_TotalSize, align_size);
		m_ArithmeticValuePositions.emplace(descName, std::make_pair(descOffset, align_size.second));
	}
}

std::unordered_map<std::string, std::pair<size_t, size_t>> const& graphics_backend::ShaderBindingSetMetadata::GetArithmeticValuePositions() const
{
	return m_ArithmeticValuePositions;
}

size_t graphics_backend::ShaderBindingSetMetadata::GetTotalSize() const
{
	return m_TotalSize;
}

std::string const& graphics_backend::ShaderBindingSetMetadata::GetSetSpaceName() const
{
	return p_Builder->GetSpaceName();
}

graphics_backend::ShaderBindingSetAllocator::ShaderBindingSetAllocator(CVulkanApplication& owner) : 
	BaseApplicationSubobject(owner)
	, m_ShaderBindingSetPool(owner)
{
}

void graphics_backend::ShaderBindingSetAllocator::Create(ShaderBindingBuilder const& builder)
{
	m_ShaderBindingSetPool.ReleaseAll();
	m_Metadata.Initialize(builder);

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	GetDevice().createDescriptorSetLayout()
}

std::shared_ptr<ShaderBindingSet> graphics_backend::ShaderBindingSetAllocator::AllocateSet()
{
	return m_ShaderBindingSetPool.AllocShared(m_Metadata);
}

void graphics_backend::ShaderBindingSetAllocator::Release()
{
	m_ShaderBindingSetPool.ReleaseAll();
}
