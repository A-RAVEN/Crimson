#pragma once
#include <deque>
#include <functional>
namespace graphics_backend
{
	template<typename T>
	class DefaultInitializer
	{
	public:
		void operator()(T* initialize)
		{
			initialize->Initialize();
		}
	};

	template<typename T>
	class DefaultReleaser
	{
	public:
		void operator()(T* release)
		{
			release->Release();
		}
	};

	template<typename T>
	class TThreadSafePointerPool
	{
	public:
		TThreadSafePointerPool(std::function<void(T*)> initializer = DefaultInitializer<T>(), std::function<void(T*)> releaser = DefaultReleaser<T>()) :
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

		void ReleaseAll()
		{
			std::lock_guard<std::mutex> lockGuard(m_Mutex);
			std::set<T*> emptySet;
			for(auto itr = m_EmptySpaces.begin(); itr != m_EmptySpaces.end(); ++itr)
			{
				emptySet.insert(*itr);
			}
			m_EmptySpaces.clear();
			std::for_each(m_Pool.begin(), m_Pool.end(), [this, &emptySet](T& itrT)
				{
					if (emptySet.find(&itrT) == emptySet.end())
					{
						m_Releaser(itrT);
					}
				});
			m_Pool.clear();
		}
	private:
		std::mutex m_Mutex;
		std::deque<T> m_Pool;
		std::deque<T*> m_EmptySpaces;
		std::function<void(T*)> m_Initializer;
		std::function<void(T*)> m_Releaser;
	};
}