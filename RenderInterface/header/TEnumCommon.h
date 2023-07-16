#pragma once
#include <type_traits>

template<class T> inline std::underlying_type_t<T> ToFlags(T a)
{
	return static_cast<std::underlying_type_t<T>>(a);
}

template<class T> inline std::underlying_type_t<T> operator~ (T a) 
{ 
	return ~static_cast<std::underlying_type_t<T>>(a);
}

/// <summary>
/// | Operators
/// </summary>
template<class T> inline std::underlying_type_t<T> operator| (T a, T b)
{ 
	return (static_cast<std::underlying_type_t<T>>(a) | static_cast<std::underlying_type_t<T>>(b));
}

template<typename T> inline std::underlying_type_t<T> operator| (T a, std::underlying_type_t<T> b)
{
	return (static_cast<std::underlying_type_t<T>>(a) | b);
}

template<typename T> inline std::underlying_type_t<T> operator| (std::underlying_type_t<T> a, T b)
{
	return (a | static_cast<std::underlying_type_t<T>>(b));
}


/// <summary>
/// & Operators
/// </summary>
template<class T> inline std::underlying_type_t<T> operator& (T a, T b)
{
	return (static_cast<std::underlying_type_t<T>>(a) & static_cast<std::underlying_type_t<T>>(b));
}

template<typename T> inline std::underlying_type_t<T> operator& (T a, std::underlying_type_t<T> b)
{
	return (static_cast<std::underlying_type_t<T>>(a) & b);
}

template<typename T> inline std::underlying_type_t<T> operator& (std::underlying_type_t<T> a, T b)
{
	return (a & static_cast<std::underlying_type_t<T>>(b));
}

/// <summary>
/// ^ Operators
/// </summary>
template<class T> inline std::underlying_type_t<T> operator^ (T a, T b)
{
	return (static_cast<std::underlying_type_t<T>>(a) ^ static_cast<std::underlying_type_t<T>>(b));
}

template<typename T> inline std::underlying_type_t<T> operator^ (T a, std::underlying_type_t<T> b)
{
	return (static_cast<std::underlying_type_t<T>>(a) ^ b);
}

template<typename T> inline std::underlying_type_t<T> operator^ (std::underlying_type_t<T> a, T b)
{
	return (a ^ static_cast<std::underlying_type_t<T>>(b));
}

/// <summary>
/// |= Operators
/// </summary>
template<class T> inline std::underlying_type_t<T>& operator|= (std::underlying_type_t<T>& a, T b)
{ 
	return (a |= static_cast<std::underlying_type_t<T>>(b));
}

/// <summary>
/// &= Operators
/// </summary>
template<class T> inline std::underlying_type_t<T>& operator&= (T& a, T b) 
{ 
	return (a &= static_cast<std::underlying_type_t<T>>(b));
}

/// <summary>
/// ^= Operators
/// </summary>
template<class T> inline std::underlying_type_t<T>& operator^= (T& a, T b) 
{ 
	return (a ^= static_cast<std::underlying_type_t<T>>(b));
}