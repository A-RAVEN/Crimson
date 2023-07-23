#pragma once
#include <SharedTools/header/uhash.h>
#include <unordered_map>
#include <private/include/VulkanApplicationSubobjectBase.h>

namespace graphics_backend
{
	template<typename DescType, typename ValType>
	class HashPool : public BaseApplicationSubobject
	{
	public:
		HashPool(CVulkanApplication& application) : BaseApplicationSubobject(application)
		{}

		ValType& Get(DescType desc)
		{
			auto it = m_InternalMap.find(desc);
			if (it == m_InternalMap.end())
			{
				ValType val{ GetVulkanApplication() };
				val.Create(desc);
				m_InternalMap.insert(std::make_pair(desc, val));
				return m_InternalMap[desc];
			}
			else
			{
				return it->second;
			}
		}

	private:
		std::unordered_map<DescType, ValType, hash_utils::default_hashAlg> m_InternalMap;
	};
}