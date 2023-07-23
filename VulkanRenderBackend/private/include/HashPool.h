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

		std::weak_ptr<ValType> Get(DescType desc)
		{
			auto it = m_InternalMap.find(desc);
			if (it == m_InternalMap.end())
			{
				std::shared_ptr<ValType> val(new ValType{ GetVulkanApplication() });
				val->Create(desc);
				m_InternalMap.insert(std::make_pair(desc, val));
				return std::weak_ptr<ValType>(m_InternalMap[desc]);
			}
			else
			{
				return std::weak_ptr<ValType>(it->second);
			}
		}

	private:
		std::unordered_map<DescType, std::shared_ptr<ValType>, hash_utils::default_hashAlg> m_InternalMap;
	};
}