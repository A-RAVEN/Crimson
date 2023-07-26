#include <iostream>
#include "library_loader.h"
#include "uhash.h"
#include <ShaderCompiler/header/Compiler.h>

namespace TestNamespace
{

	struct TestStruct
	{
	public:
		int32_t one;
		int32_t two;
	};

	enum class TestEnum : uint8_t
	{
		e0 = 0,
		e1,
		e5,
		e2,
		e3
	};

	struct TestStruct1
	{
	public:
		int32_t one;
		int32_t two;
		int32_t three;
		template <class HashAlgorithm>
		friend void hash_append(HashAlgorithm& h, TestStruct1 const& x) noexcept
		{
			hash_append(h, x.one);
			hash_append(h, x.two);
			hash_append(h, x.three);
		}
	};

}

template<>
struct ::hash_utils::is_contiguously_hashable<TestNamespace::TestStruct> : public std::true_type {};

int main(int argc, char* argv[])
{
	TestNamespace::TestStruct test = { 512, 1024 };
	TestNamespace::TestStruct test1 = { 1024, 512 };

	std::string testStr = "aaabbcc";

	TestNamespace::TestStruct1	test2 = { 5,2,7 };

	std::pair<int, int> testpair = std::make_pair(5, 6);

	TestNamespace::TestEnum en = TestNamespace::TestEnum::e1;

	hash_utils::uhash<hash_utils::fnv1a> hash_generator;
	std::cout << hash_generator(test) << std::endl;
	std::cout << hash_generator(test1) << std::endl;
	std::cout << hash_generator(testStr) << std::endl;
	std::cout << hash_generator(testpair) << std::endl;
	std::cout << hash_generator(en) << std::endl;
	std::cout << hash_generator(test2) << std::endl;

	library_loader::TModuleLoader<ShaderCompiler::IShaderCompiler> compilerLoader(L"ShaderCompiler");
	auto compiler = compilerLoader.New();
	return EXIT_SUCCESS;
}