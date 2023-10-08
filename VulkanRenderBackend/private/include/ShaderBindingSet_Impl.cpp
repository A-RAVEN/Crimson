#include "private/include/pch.h"
#include "CVulkanApplication.h"
#include "ShaderBindingSet_Impl.h"

namespace graphics_backend {

	//align, size
	std::pair<size_t, size_t> GetVectorAlignment(uint32_t vector_size)
	{
		switch (vector_size)
		{
		case 1:
			return std::pair<size_t, size_t>{ 1, 1 };
		case 2:
			return std::pair<size_t, size_t>{ 2, 2 };
		case 3:
			return std::pair<size_t, size_t>{ 4, 3 };
		case 4:
			return std::pair<size_t, size_t>{ 4, 4 };
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

	ShaderConstantSet_Impl::ShaderConstantSet_Impl(CVulkanApplication& owner) :
		BaseUploadingResource(owner)
	{
	}

	void ShaderConstantSet_Impl::SetValue(std::string const& name, void* pValue)
	{
		auto& positions = p_Metadata->GetArithmeticValuePositions();
		auto found = positions.find(name);
		if (found == positions.end())
		{
			CA_LOG_ERR("shader variable " + name + " not found in constant buffer");
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

	void ShaderConstantSet_Impl::Initialize(ShaderConstantSetMetadata const* inMetaData)
	{
		p_Metadata = inMetaData;
	}

	void ShaderConstantSet_Impl::UploadAsync()
	{
		BaseUploadingResource::UploadAsync(UploadingResourceType::eAddressDataThisFrame);
	}

	bool ShaderConstantSet_Impl::UploadingDone() const
	{
		return BaseUploadingResource::UploadingDone();
	}

	std::string const& ShaderConstantSet_Impl::GetName() const
	{
		return p_Metadata->GetBuilder()->GetName();
	}

	void ShaderConstantSet_Impl::DoUpload()
	{
		CVulkanMemoryManager& memoryManager = GetMemoryManager();
		auto threadContext = GetVulkanApplication().AquireThreadContextPtr();
		auto currentFrame = GetFrameCountContext().GetCurrentFrameID();

		size_t bufferSize = p_Metadata->GetTotalSize() * sizeof(uint32_t);
		if (m_BufferObject == nullptr)
		{
			m_BufferObject = memoryManager.AllocateBuffer(
				EMemoryType::GPU
				, EMemoryLifetime::Persistent
				, bufferSize
				, vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst);
		}
		auto tempBuffer = memoryManager.AllocateFrameBoundTransferStagingBuffer(bufferSize);
		memcpy(tempBuffer->GetMappedPointer(), m_UploadData.data(), bufferSize);

		auto cmdBuffer = threadContext->GetCurrentFramePool().AllocateMiscCommandBuffer("Upload GPU Buffer");
		cmdBuffer.copyBuffer(tempBuffer->GetBuffer(), m_BufferObject->GetBuffer(), vk::BufferCopy(0, 0, bufferSize));
		cmdBuffer.end();
		std::atomic_thread_fence(std::memory_order_release);
		MarkUploadingDoneThisFrame();
	}

	size_t AccumulateOnOffset(size_t& inoutOffset, std::pair<size_t, size_t> const& inAlign_Size)
	{
		size_t next_offset = (inoutOffset + inAlign_Size.first - 1) / inAlign_Size.first * inAlign_Size.first;
		inoutOffset = next_offset + inAlign_Size.second;
		return next_offset;
	}

	void ShaderConstantSetMetadata::Initialize(ShaderConstantsBuilder const& builder)
	{
		p_Builder = &builder;
		auto& descriptors = builder.GetNumericDescriptors();
		m_ArithmeticValuePositions.clear();
		m_TotalSize = 0;
		for (auto& descPairs : descriptors)
		{
			auto& desc = descPairs.second;
			auto& descName = descPairs.first;
			uint32_t count = desc.count;
			CA_ASSERT(count > 0
				, "descriptor must have at least one element");
			CA_ASSERT(m_ArithmeticValuePositions.find(descName) == m_ArithmeticValuePositions.end()
				, "descriptor must have at least one element");
			std::pair<size_t, size_t>align_size;
			if (desc.y == 1)
			{
				uint32_t vectorSize = desc.x * desc.y;
				if (count == 1)
				{
					align_size = GetVectorAlignment(vectorSize);
				}
				else
				{
					align_size = std::make_pair(4, 4 * count);
				}
			}
			else
			{
				//Matrix always align at 4
				uint32_t vectorSize = desc.x;
				align_size = std::make_pair(4, 4 * count * desc.y);
			}

			CA_ASSERT(align_size.first > 0 && align_size.second > 0
				, "invalid align size");
			size_t descOffset = AccumulateOnOffset(m_TotalSize, align_size);
			m_ArithmeticValuePositions.emplace(descName, std::make_pair(descOffset, align_size.second));
		}
	}

	std::unordered_map<std::string, std::pair<size_t, size_t>> const& ShaderConstantSetMetadata::GetArithmeticValuePositions() const
	{
		return m_ArithmeticValuePositions;
	}

	size_t ShaderConstantSetMetadata::GetTotalSize() const
	{
		return m_TotalSize;
	}

	ShaderConstantSetAllocator::ShaderConstantSetAllocator(CVulkanApplication& owner) :
		BaseApplicationSubobject(owner)
		, m_ShaderConstantSetPool(owner)
	{
	}

	void ShaderConstantSetAllocator::Create(ShaderConstantsBuilder const& builder)
	{
		m_ShaderConstantSetPool.ReleaseAll();
		m_Metadata.Initialize(builder);
	}

	std::shared_ptr<ShaderConstantSet> ShaderConstantSetAllocator::AllocateSet()
	{
		return m_ShaderConstantSetPool.AllocShared(&m_Metadata);
	}

	void ShaderConstantSetAllocator::Release()
	{
		m_ShaderConstantSetPool.ReleaseAll();
	}
	ShaderBindingSet_Impl::ShaderBindingSet_Impl(CVulkanApplication& owner) : BaseTickingUpdateResource(owner)
	{
	}
	void ShaderBindingSet_Impl::Initialize(ShaderBindingSetMetadata const* inMetaData)
	{
		p_Metadata = inMetaData;
	}
	void ShaderBindingSet_Impl::SetConstantSet(std::string const& name, std::shared_ptr<ShaderConstantSet> const& pConstantSet)
	{
		m_ConstantSets.insert(std::make_pair(name, pConstantSet));
		MarkDirtyThisFrame();
	}

	void ShaderBindingSet_Impl::SetTexture(std::string const& name
		, std::shared_ptr<GPUTexture> const& pTexture)
	{
		std::shared_ptr<GPUTexture_Impl> texture = std::static_pointer_cast<GPUTexture_Impl>(pTexture);
		m_Textures.insert(std::make_pair(name, texture));
		MarkDirtyThisFrame();
	}

	void ShaderBindingSet_Impl::SetSampler(std::string const& name
		, std::shared_ptr<TextureSampler> const& pSampler)
	{
		std::shared_ptr<TextureSampler_Impl> sampler = std::static_pointer_cast<TextureSampler_Impl>(pSampler);
		m_Samplers.insert(std::make_pair(name, sampler));
		MarkDirtyThisFrame();
	}

	bool ShaderBindingSet_Impl::UploadingDone() const
	{
		return BaseTickingUpdateResource::UploadingDone();
	}
	void ShaderBindingSet_Impl::TickUpload()
	{
		m_DescriptorSetHandle.RAIIRelease();
		CA_ASSERT(!m_DescriptorSetHandle.IsRAIIAquired(), "RAIIObject Should Be Released Here");
		//{
			auto& descPoolCache = GetVulkanApplication().GetGPUObjectManager().GetShaderDescriptorPoolCache();
			ShaderDescriptorSetLayoutInfo layoutInfo = p_Metadata->GetLayoutInfo();
			auto allocator = descPoolCache.GetOrCreate(layoutInfo).lock();
			m_DescriptorSetHandle = std::move(allocator->AllocateSet());
		//}
		CA_ASSERT(m_DescriptorSetHandle.IsRAIIAquired(), "Descriptor Set Is Not Aquired!");
		vk::DescriptorSet targetSet = m_DescriptorSetHandle->GetDescriptorSet();
		uint32_t writeCount = m_ConstantSets.size() + m_Textures.size() + m_Samplers.size();
		std::vector<vk::WriteDescriptorSet> descriptorWrites;
		if(writeCount > 0)
		{
			descriptorWrites.reserve(writeCount);
			auto& nameToIndex = p_Metadata->GetCBufferNameToBindingIndex();
			for (auto pair : m_ConstantSets)
			{
				auto& name = pair.first;
				auto set = std::static_pointer_cast<ShaderConstantSet_Impl>(pair.second);
				uint32_t bindingIndex = p_Metadata->CBufferNameToBindingIndex(name);
				if (bindingIndex != std::numeric_limits<uint32_t>::max())
				{
					vk::DescriptorBufferInfo bufferInfo{set->GetBufferObject()->GetBuffer(), 0, VK_WHOLE_SIZE};
					vk::WriteDescriptorSet writeSet{targetSet
						, bindingIndex
						, 0
						, 1
						, vk::DescriptorType::eUniformBuffer
						, nullptr
						, & bufferInfo
						, nullptr
					};
					descriptorWrites.push_back(writeSet);
				}
			}

			for (auto pair : m_Textures)
			{
				auto& name = pair.first;
				auto& texture = pair.second;
				uint32_t bindingIndex = p_Metadata->TextureNameToBindingIndex(name);
				if (bindingIndex != std::numeric_limits<uint32_t>::max())
				{
					if (!texture->UploadingDone())
						return;
					vk::DescriptorImageInfo imageInfo{ {}
						, texture->GetDefaultImageView()
						, vk::ImageLayout::eShaderReadOnlyOptimal };

					vk::WriteDescriptorSet writeSet{targetSet
						, bindingIndex
						, 0
						, 1
						, vk::DescriptorType::eSampledImage
						, & imageInfo
						, nullptr
						, nullptr
					};
					descriptorWrites.push_back(writeSet);
				}
			}

			for (auto pair : m_Samplers)
			{
				auto& name = pair.first;
				auto& sampler = pair.second;
				uint32_t bindingIndex = p_Metadata->SamplerNameToBindingIndex(name);
				if (bindingIndex != std::numeric_limits<uint32_t>::max())
				{
					vk::DescriptorImageInfo samplerInfo{ sampler->GetSampler()
						, {}
						, vk::ImageLayout::eShaderReadOnlyOptimal };

					vk::WriteDescriptorSet writeSet{targetSet
						, bindingIndex
						, 0
						, 1
						, vk::DescriptorType::eSampler
						, & samplerInfo
						, nullptr
						, nullptr
					};
					descriptorWrites.push_back(writeSet);
				}
			}
		}
		if (descriptorWrites.size() > 0)
		{
			GetDevice().updateDescriptorSets(descriptorWrites, {});
		}
		MarkUploadingDoneThisFrame();
	}
	void ShaderBindingSetMetadata::Initialize(ShaderBindingBuilder const& builder)
	{
		auto& constantBufferDescriptors = builder.GetConstantBufferDescriptors();
		auto& textureDescriptors = builder.GetTextureDescriptors();
		auto& samplerDescriptors = builder.GetTextureSamplers();
		m_LayoutInfo = ShaderDescriptorSetLayoutInfo{ builder };

		uint32_t bindingIndex = 0;
		for (auto& desc : constantBufferDescriptors)
		{
			m_CBufferNameToBindingIndex.emplace(desc.GetName(), bindingIndex++);
		};

		for (auto& desc : textureDescriptors)
		{
			m_TextureNameToBindingIndex.emplace(desc.first, bindingIndex++);
		}

		for (auto& sampler : samplerDescriptors)
		{
			m_SamplerNameToBindingIndex.emplace(sampler, bindingIndex++);
		}
	}
	ShaderBindingSetAllocator::ShaderBindingSetAllocator(CVulkanApplication& owner) : BaseApplicationSubobject(owner)
		, m_ShaderBindingSetPool(owner)
	{
	}
	void ShaderBindingSetAllocator::Create(ShaderBindingBuilder const& builder)
	{
		m_Metadata.Initialize(builder);
	}
	std::shared_ptr<ShaderBindingSet> ShaderBindingSetAllocator::AllocateSet()
	{
		return m_ShaderBindingSetPool.AllocShared(&m_Metadata);
	}
	void ShaderBindingSetAllocator::Release()
	{
		m_ShaderBindingSetPool.ReleaseAll();
	}
	void ShaderBindingSetAllocator::TickUploadResources(CTaskGraph* pTaskGraph)
	{
		m_ShaderBindingSetPool.TickUpload(pTaskGraph);
	}
}
