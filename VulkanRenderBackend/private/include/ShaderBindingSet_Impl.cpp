#include "private/include/pch.h"
#include "ShaderBindingSet_Impl.h"

//align, size
std::pair<size_t, size_t> GetVectorAlignment(uint32_t vector_size)
{
	switch (vector_size)
	{
	case 1:
		return { 1, 1 };
	case 2:
		return { 2, 2 };
	case 3:
		return { 4, 3 };
	case 4:
		return { 4, 4 };
	default:
		CA_LOG_ERR(std::string("UnSupported Vector Size: %d", vector_size));
	}
	return { 0, 0 };
}

//Column Major
std::pair<size_t, size_t> GetMatrixAlignment(uint32_t column, uint32_t row)
{
	return { 4, 4 * row };
}

void graphics_backend::ShaderBindingSet_Impl::SetArithmeticValue(std::string const& name, void* pValue)
{

}

void graphics_backend::ShaderBindingSetMetadata::Initialize(ShaderBindingBuilder const& builder)
{
	auto& descriptors = builder.GetNumericDescriptors();
	m_ArithmeticValuePositions.clear();
	size_t offset = 0;
	for (auto& desc : descriptors)
	{

	}
}
