#pragma once
#include <mutex>
#include <deque>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include "VulkanApplicationSubobjectBase.h"
namespace graphics_backend
{
	template<typename T>
	class TVulkanApplicationPool
	{
	public:
		TVulkanApplicationPool() = delete;
		TVulkanApplicationPool(TVulkanApplicationPool const& other) = delete;
		TVulkanApplicationPool& operator=(TVulkanApplicationPool const&) = delete;
		TVulkanApplicationPool(TVulkanApplicationPool&& other) = delete;
		TVulkanApplicationPool & operator=(TVulkanApplicationPool&&) = delete;

		TVulkanApplicationPool(CVulkanApplication& owner) :
			m_Owner(owner)
		{
		}

		virtual ~TVulkanApplicationPool()
		{
			CA_ASSERT(IsEmpty(), std::string{"Vulkan Application Pointer Pool Is Not Released Before Destruct: "} + CA_CLASS_NAME(T));
		}

		template<typename...TArgs>
		T* Alloc(TArgs&&...Args)
		{
			std::lock_guard<std::mutex> lockGuard(m_Mutex);
			T* result = nullptr;
			if (m_EmptySpaces.empty())
			{
				m_Pool.emplace_back(m_Owner);
				result = &m_Pool.back();
			}
			else
			{
				result = m_EmptySpaces.front();
				m_EmptySpaces.pop_front();
			}
			result->Initialize(std::forward<TArgs>(Args)...);
			return result;
		}

		void Release(T* releaseObj)
		{
			CA_ASSERT(releaseObj != nullptr, std::string{"Try Release nullptr: "} + CA_CLASS_NAME(T));
			std::lock_guard<std::mutex> lockGuard(m_Mutex);
			releaseObj->Release();
			m_EmptySpaces.push_back(releaseObj);
		}

		void ReleaseAll()
		{
			std::lock_guard<std::mutex> lockGuard(m_Mutex);

			std::unordered_set<T*> emptySet;
			for (T* emptyPtr : m_EmptySpaces)
			{
				emptySet.insert(emptyPtr);
			}

			for (size_t i = 0; i < m_Pool.size(); ++i)
			{
				if (emptySet.find(&m_Pool[i]) == emptySet.end())
				{
					m_Pool[i].Release();
				}
			}
			m_Pool.clear();
			emptySet.clear();
			m_EmptySpaces.clear();
		}

		template<typename...TArgs>
		std::shared_ptr<T> AllocShared(TArgs&&...Args)
		{
			return std::shared_ptr<T>(Alloc(std::forward<TArgs>(Args)...), [this](T* releaseObj) { Release(releaseObj); });
		}

		bool IsEmpty()
		{
			std::lock_guard<std::mutex> lockGuard(m_Mutex);
			return m_EmptySpaces.size() == m_Pool.size();
		}
	private:
		CVulkanApplication& m_Owner;
		std::mutex m_Mutex;
		std::deque<T> m_Pool;
		std::deque<T*> m_EmptySpaces;
	};

	template<typename Key, typename T>
	class TVulkanValuePool : public BaseApplicationSubobject
	{
	public:
		TVulkanValuePool(CVulkanApplication& owner) :
			BaseApplicationSubobject(owner)
		{
		}

		template<typename...TArgs>
		T* Alloc(Key const& key, TArgs&&...Args)
		{
			std::lock_guard<std::mutex> lockGuard(m_Mutex);
			auto found = m_Pools.find(key);
			if (found == m_Pools.end())
			{
				m_Pools.emplace(key, TVulkanApplicationPool<T>{GetVulkanApplication()});
				found = m_Pools.find(key);
			};
			return found->second.Alloc(std::forward<TArgs>(Args)...);
		}

		void Release(Key const& key, T* releaseObj)
		{
			CA_ASSERT(releaseObj != nullptr, std::string{"Try Release nullptr: "} + CA_CLASS_NAME(T));
			std::lock_guard<std::mutex> lockGuard(m_Mutex);
			auto found = m_Pools.find(key);
			if (found != m_Pools.end())
			{
				found->second.Release(releaseObj);
			};
		}

		template<typename...TArgs>
		std::shared_ptr<T> AllocShared(Key const& key, TArgs&&...Args)
		{
			return std::shared_ptr<T>(Alloc(key, std::forward<TArgs>(Args)...), [this, key](T* releaseObj) { Release(key, releaseObj); });
		}

		void ReleaseAll()
		{
			std::lock_guard<std::mutex> lockGuard(m_Mutex);

			for (auto& itr = m_Pools.begin(), itr != m_Pools.end(); ++itr)
			{
				itr->second.ReleaseAll();
			}
			m_Pools.clear();
		}
	private:
		std::mutex m_Mutex;
		std::unordered_map<Key, TVulkanApplicationPool<T>> m_Pools;
	};
}