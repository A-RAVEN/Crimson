#pragma once
#include <mutex>
#include <deque>
#include <functional>
#include "DebugUtils.h"

namespace threadsafe_utils
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
		TThreadSafePointerPool() = delete;
		TThreadSafePointerPool(TThreadSafePointerPool const& other) = delete;
		TThreadSafePointerPool& operator=(TThreadSafePointerPool const&) = delete;
		TThreadSafePointerPool(TThreadSafePointerPool&& other) = delete;
		TThreadSafePointerPool& operator=(TThreadSafePointerPool&&) = delete;

		TThreadSafePointerPool(std::function<void(T*)> initializer = DefaultInitializer<T>{}, std::function<void(T*)> releaser = DefaultReleaser<T>{}) :
			m_Initializer(initializer)
			, m_Releaser(releaser)
		{
		}
		virtual ~TThreadSafePointerPool()
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
			std::lock_guard<std::mutex> lockGuard(m_Mutex);
			m_EmptySpaces.push_back(releaseObj);
			m_Releaser(releaseObj);
		}

		bool IsEmpty()
		{
			std::lock_guard<std::mutex> lockGuard(m_Mutex);
			return m_EmptySpaces.size() == m_Pool.size();
		}
	protected:
		std::mutex m_Mutex;
		std::deque<T> m_Pool;
		std::deque<T*> m_EmptySpaces;
		std::function<void(T*)> m_Initializer;
		std::function<void(T*)> m_Releaser;
	};
}