#pragma once
#include <type_traits>
namespace shared_tools
{
	template<typename T>
	class TMoveWrapper
	{
	public:
		explicit TMoveWrapper(T&& value) :m_Value(std::move(value)) {}
		TMoveWrapper& operator=(TMoveWrapper const& other) = delete;
		TMoveWrapper& operator=(TMoveWrapper&& other) = delete;
		TMoveWrapper(const TMoveWrapper& other) : m_Value(std::move(other.m_Value)) {}
		TMoveWrapper(TMoveWrapper&& other) : m_Value(std::move(other.m_Value)) {}
		T&& Move() { return std::move(m_Value); }

		const T& operator*() const { return m_Value; }
		T& operator*() { return m_Value; }
		const T* operator->() const { return &m_Value; }
		T* operator->() { return &m_Value; }
	private:
		mutable T m_Value;
	};

	template <class T, class T0 = std::remove_reference_t<T>>
	TMoveWrapper<T0> makeMoveWrapper(T&& t) {
		return TMoveWrapper<T0>(std::forward<T0>(t));
	}
}