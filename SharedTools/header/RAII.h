#pragma once
#include <functional>

namespace raii_utils
{
	template<typename T>
	class TRAIIContainer
	{
	public:
		TRAIIContainer(T&& rttiData, std::function<void(T& releaseObj)> releaser) : m_RAIIData(rttiData)
			, _Releaser(releaser)
			, _RAII_Aquired(true)
		{
		}

		void RAIIRelease()
		{
			if (!_RAII_Aquired)
				return;
			_RAII_Aquired = false;
			_Releaser(m_RAIIData);
		}

		virtual ~TRAIIContainer()
		{
			RAIIRelease();
		}

		TRAIIContainer(TRAIIContainer const& other) = delete;
		TRAIIContainer& operator=(TRAIIContainer const& other) = delete;

		TRAIIContainer(TRAIIContainer&& other) : 
			m_RAIIData(std::move(other.m_RAIIData))
			, _Releaser(std::move(other._Releaser))
			, _RAII_Aquired(other._RAII_Aquired)
		{
			other._RAII_Aquired = false;
		}

		T& Get()
		{
			return m_RAIIData;
		}

		T const& Get() const
		{
			return m_RAIIData;
		}

		T& operator*()
		{
			return Get();
		}

		T const& operator* const()
		{
			return Get();
		}

		template<typename TT = T>
		typename std::enable_if_t<std::is_pointer_v<TT>, TT> operator->() const noexcept
		{
			return m_RAIIData;
		}

		template<typename TT = T>
		typename std::enable_if_t<std::is_class_v<TT>, TT> operator->() const noexcept
		{
			return std::addressof(m_RAIIData);
		}

	private:
		T m_RAIIData;
		std::function<void(T& releaseObj)> _Releaser;
		bool _RAII_Aquired = false;
	};
}