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

		HashPool() = delete;
		HashPool(HashPool const& other) = delete;
		HashPool& operator=(HashPool const&) = delete;
		HashPool(HashPool&& other) = delete;
		HashPool& operator=(HashPool&&) = delete;

		HashPool(CVulkanApplication& application) : BaseApplicationSubobject(application)
		{}

		std::weak_ptr<ValType> GetOrCreate(DescType desc)
		{
			std::shared_ptr<ValType> result;
			bool newResult = false;
			{
				std::lock_guard<std::mutex> lockGuard(m_Mutex);
				auto it = m_InternalMap.find(desc);
				newResult = it == m_InternalMap.end();
				if (newResult)
				{
					result = std::shared_ptr<ValType>{ new ValType{ GetVulkanApplication() } };
					m_InternalMap.insert(std::make_pair(desc, result));
					it = m_InternalMap.find(desc);
					result = it->second;
				}
				else
				{
					result = it->second;
				}
				if (newResult)
				{
					result->Create(it->first);
				}
			}
			return std::weak_ptr<ValType>(result);
		}

	private:
		std::mutex m_Mutex;
		std::unordered_map<DescType, std::shared_ptr<ValType>, hash_utils::default_hashAlg> m_InternalMap;
	};
}