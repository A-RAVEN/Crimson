#include <iostream>

#include "uhash.h"

struct TestStruct
{
public:
	int32_t one;
	int32_t two;
	//template <class HashAlgorithm>
	//friend void hash_append(HashAlgorithm& h, TestStruct const& x) noexcept
	//{
	//	hash_append(h, x.one);
	//	hash_append(h, x.two);
	//}
};

enum class TestEnum : uint8_t
{
	e0 = 0,
	e1,
	e5,
	e2,
	e3
};

template<>
struct is_contiguously_hashable<TestStruct> : public std::true_type{};

int main(int argc, char* argv[])
{
	TestStruct test = { 512, 1024 };
	TestStruct test1 = { 1024, 512 };

	std::string testStr = "aaabbcc";

	std::pair<int, int> testpair = std::make_pair(5, 6);

	TestEnum en = TestEnum::e1;

	uhash<fnv1a> hash_generator;
	std::cout << hash_generator(test) << std::endl;
	std::cout << hash_generator(test1) << std::endl;
	std::cout << hash_generator(testStr) << std::endl;
	std::cout << hash_generator(testpair) << std::endl;
	std::cout << hash_generator(en) << std::endl;
	return EXIT_SUCCESS;
}