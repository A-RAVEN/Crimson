#pragma once
#include <deque>
#include <vector>
#include <memory>

//在需要hash的类或结构体中实现hash_append，在其中自行将需要计算hash的成员变量累加
//template <class HashAlgorithm>
//friend void hash_append(HashAlgorithm& h, TestStruct const& x) noexcept
//{
//	hash_append(h, x.one);
//	hash_append(h, x.two);
//}

//如果结构体本身是连续的内存（有指针和堆内存容器）可以声明一个is contiguous
//template<>
//struct is_contiguously_hashable<TestStruct> : public std::true_type {};

namespace hash_utils
{
	

#pragma region Is Contiguously

template <class T> struct is_contiguously_hashable : public std::integral_constant<bool, std::is_integral<T>::value || std::is_enum<T>::value> {};

template <class T, class U>
struct is_contiguously_hashable<std::pair<T, U>>
    : public std::integral_constant<bool, is_contiguously_hashable<T>::value&&
    is_contiguously_hashable<U>::value &&
    sizeof(T) + sizeof(U) == sizeof(std::pair<T, U>)>
{
};

#pragma endregion

#pragma region Hash Algorithms
class fnv1a
{
    size_t state_ = 14695981039346656037u;
public:
    using result_type = size_t;

    void
        operator()(void const* key, size_t len) noexcept
    {
        unsigned char const* p = static_cast<unsigned char const*>(key);
        unsigned char const* const e = p + len;
        for (; p < e; ++p)
            state_ = (state_ ^ *p) * 1099511628211u;
    }

    explicit
        operator result_type() noexcept
	    {
	        return state_;
	    }
};


#pragma endregion

#pragma region Hash Functor
template <class HashAlgorithm>
struct uhash
{
    using result_type = typename HashAlgorithm::result_type;

    template <class T>
    result_type
        operator()(T const& t) const noexcept
    {
        HashAlgorithm h;
        hash_append(h, t);
        return static_cast<result_type>(h);
    }
};

using default_hashAlg = uhash<fnv1a>;
#pragma endregion

#pragma region contiguous hash_append
template <class HashAlgorithm, class T>
inline
std::enable_if_t
<
    is_contiguously_hashable<T>::value
>
hash_append(HashAlgorithm& h, T const& t) noexcept
{
    h(std::addressof(t), sizeof(t));
}
#pragma endregion

#pragma region vector hash_append
template <class HashAlgorithm, class T, class Alloc>
inline
std::enable_if_t
<
    !is_contiguously_hashable<T>::value
>
hash_append(HashAlgorithm& h, std::vector<T, Alloc> const& v) noexcept
{
    for (auto const& t : v)
        hash_append(h, t);
    hash_append(h, v.size());
}

template <class HashAlgorithm, class T, class Alloc>
inline
std::enable_if_t
<
    is_contiguously_hashable<T>::value
>
hash_append(HashAlgorithm& h, std::vector<T, Alloc> const& v) noexcept
{
    h(v.data(), v.size() * sizeof(T));
    hash_append(h, v.size());
}
#pragma endregion

#pragma region array hash_append
template <class HashAlgorithm, class T, size_t Size>
inline
std::enable_if_t
<
    !is_contiguously_hashable<T>::value
>
hash_append(HashAlgorithm& h, std::array<T, Size> const& arr) noexcept
{
    for (auto const& t : arr)
        hash_append(h, t);
    hash_append(h, arr.size());
}

template <class HashAlgorithm, class T, size_t Size>
inline
std::enable_if_t
<
    is_contiguously_hashable<T>::value
>
hash_append(HashAlgorithm& h, std::array<T, Size> const& arr) noexcept
{
    h(arr.data(), arr.size() * sizeof(T));
    hash_append(h, arr.size());
}
#pragma endregion

#pragma region deque hash_append
template <class HashAlgorithm, class T, class Alloc>
void
hash_append(HashAlgorithm& h, std::deque<T, Alloc> const& v) noexcept
{
    for (auto const& t : v)
        hash_append(h, t);
    hash_append(h, v.size());
}
#pragma endregion

#pragma region string hash_append
template <class HashAlgorithm, class CharT, class Traits, class Alloc>
std::enable_if_t
<
    !is_contiguously_hashable<CharT>::value
>
hash_append(HashAlgorithm& h, std::basic_string<CharT, Traits, Alloc> const& s) noexcept
{
    for (auto c : s)
        hash_append(h, c);
    hash_append(h, s.size());
}

template <class HashAlgorithm, class CharT, class Traits, class Alloc>
std::enable_if_t
<
    is_contiguously_hashable<CharT>::value
>
hash_append(HashAlgorithm& h, std::basic_string<CharT, Traits, Alloc> const& s) noexcept
{
    h(s.data(), s.size() * sizeof(CharT));
}
#pragma endregion

#pragma region pointer hash_append
template <class HashAlgorithm, class T>
void hash_append(HashAlgorithm& h, T* const pT) noexcept
{
    if(pT == nullptr)
    {
        hash_append(h, size_t{0});
    }
    else
    {
        hash_append(h, *pT);
    }
}

template <class HashAlgorithm, class T>
void hash_append(HashAlgorithm& h, std::shared_ptr<T> const pT) noexcept
{
    if (pT == nullptr)
    {
        hash_append(h, size_t{ 0 });
    }
    else
    {
        hash_append(h, reinterpret_cast<size_t>(pT.get()));
    }
}
#pragma endregion

#pragma region basic types hash_append
template <class HashAlgorithm, class T>
inline
std::enable_if_t
<
    std::is_floating_point<T>::value
>
hash_append(HashAlgorithm& h, T t) noexcept
{
    if (t == 0)
        t = 0;
    h(&t, sizeof(t));
}

template <class HashAlgorithm, class T, class U>
std::enable_if_t
<
    !is_contiguously_hashable<std::pair<T, U>>::value
>
hash_append(HashAlgorithm& h, std::pair<T, U> const& p) noexcept
{
    hash_append(h, p.first);
    hash_append(h, p.second);
}


#pragma endregion

#pragma region
template<typename T>
bool memory_equal(T const& lhs, T const& rhs) noexcept
{
	return std::memcmp(&lhs, &rhs, sizeof(T)) == 0;
};

#pragma endregion
}
