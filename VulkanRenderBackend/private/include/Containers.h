#pragma once
#include <deque>
#include <functional>
#include <SharedTools/header/ThreadSafePool.h>
namespace graphics_backend
{
	template<typename T>
	class TVulkanApplicationPool
	{
	public:
		TVulkanApplicationPool() = delete;
		TVulkanApplicationPool(TVulkanApplicationPool const& other) = delete;
		TVulkanApplicationPool& operator=(TVulkanApplicationPool const&) = delete;
		TVulkanApplicationPool(TVulkanApplicationPool&& other) = delete
		TVulkanApplicationPool & operator=(TVulkanApplicationPool&&) = delete;

		TVulkanApplicationPool(CVulkanApplication& owner) :
			m_Owner(owner)
		{
		}

		virtual ~TVulkanApplicationPool()
		{
			CA_ASSERT(IsEmpty(), std::string{"ThreadSafe Pointer Pool Is Not Released Before Destruct: "} + CA_CLASS_NAME(T));
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
			result->Release();
			std::lock_guard<std::mutex> lockGuard(m_Mutex);
			m_EmptySpaces.push_back(releaseObj);
		}

		template<typename...TArgs>
		std::shared_ptr<T> AllocShared(TArgs&&...Args)
		{
			return std::shared_ptr<T>(Alloc(std::forward<TArgs>(Args)...), [this](T* releaseObj) { Release(releaseObj); });
		}

		bool IsEmpty() const
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
}