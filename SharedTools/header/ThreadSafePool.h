#pragma once
#include <mutex>
#include <deque>
#include <functional>

namespace threadsafe_utils
{
	template<typename T>
	class TThreadSafePointerPool
	{
	public:
		TThreadSafePointerPool(std::function<void(T*)> initializer, std::function<void(T*)> releaser) :
			m_Initializer(initializer)
			, m_Releaser(releaser)
		{
		}
		template<typename...TArgs>
		T* Alloc(TArgs&&...Args)
		{
			std::lock_guard<std::mutex> lockGuard(m_Mutex);
			T* result = nullptr;
			if (m_EmptySpaces.empty())
			{
				m_Pool.emplace_back(std::forward<TArgs>(Args)...);
				result = &m_Pool.back();
			}
			else
			{
				result = m_EmptySpaces.front();
				m_EmptySpaces.pop_front();
			}
			m_Initializer(result);
			return result;
		}

		void Release(T* releaseObj)
		{
			assert(releaseObj != nullptr);
			m_Releaser(releaseObj);
			std::lock_guard<std::mutex> lockGuard(m_Mutex);
			m_EmptySpaces.push_back(releaseObj);
		}
	private:
		std::mutex m_Mutex;
		std::deque<T> m_Pool;
		std::deque<T*> m_EmptySpaces;
		std::function<void(T*)> m_Initializer;
		std::function<void(T*)> m_Releaser;
	};
}