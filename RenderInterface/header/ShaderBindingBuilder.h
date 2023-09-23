#pragma once
#include <string>
#include <map>
#include <vector>
#include <SharedTools/header/uhash.h>

enum class EShaderBindingNumericType
{
	eFloat = 0,
	eInt32,
	eUInt32,
};

enum class ETextureDimension
{
	e2D = 0,
	e3D
};

template <class T>
struct is_shaderbinding_arighmetic_type : public std::integral_constant<bool, (std::is_integral<T>::value && sizeof(T) == 4)> 
{
public:
	static constexpr EShaderBindingNumericType numericType = EShaderBindingNumericType::eUInt32;
};

template <>
struct is_shaderbinding_arighmetic_type<float> : public std::true_type
{
public:
	static constexpr EShaderBindingNumericType numericType = EShaderBindingNumericType::eFloat;
};

template <>
struct is_shaderbinding_arighmetic_type<int32_t> : public std::true_type
{
public:
	static constexpr EShaderBindingNumericType numericType = EShaderBindingNumericType::eInt32;
};

struct ShaderBindingDescriptor
{
public:
	EShaderBindingNumericType numericType;
	uint32_t count = 1;
	uint32_t x = 1;
	uint32_t y = 1;
	ShaderBindingDescriptor(EShaderBindingNumericType inType
		, uint32_t inX = 1, uint32_t inY = 1, uint32_t inCount = 1) :
		numericType(inType)
		, count(inCount)
		, x(inX)
		, y(inY)
	{
	}
	bool operator==(ShaderBindingDescriptor const& other) const
	{
		return hash_utils::memory_equal(*this, other);
	}
};

template<>
struct hash_utils::is_contiguously_hashable<ShaderBindingDescriptor> : public std::true_type {};

struct ShaderTextureDescriptor
{
public:
	EShaderBindingNumericType numericType;
	uint32_t channels = 4;
	uint32_t count = 1;
	ETextureDimension dimension = ETextureDimension::e2D;
	bool isRW = false;
	ShaderTextureDescriptor(EShaderBindingNumericType inType
		, bool inIsRW, ETextureDimension inDim, uint32_t inChannel = 4, uint32_t inCount = 1) :
		numericType(inType)
		, channels(inChannel)
		, count(inCount)
		, dimension(inDim)
		, isRW(inIsRW)
	{
	}

	bool operator==(ShaderTextureDescriptor const& other) const
	{
		return hash_utils::memory_equal(*this, other);
	}
};

template<>
struct hash_utils::is_contiguously_hashable<ShaderTextureDescriptor> : public std::true_type {};

class ShaderConstantsBuilder
{
public:
	ShaderConstantsBuilder(std::string const& name) : m_Name(name){}

	template<typename T>
	ShaderConstantsBuilder& Scalar(std::string const& name)
	{
		static_assert(is_shaderbinding_arighmetic_type<T>::value, "shader binding arighmetic type only support 32 bit arithmetic type");
		m_NumericDescriptors.push_back(std::make_pair(name, ShaderBindingDescriptor{ 
			is_shaderbinding_arighmetic_type<T>::numericType, 1 }));
		return *this;
	}

	template<typename T>
	ShaderConstantsBuilder& Vec2(std::string const& name)
	{
		static_assert(is_shaderbinding_arighmetic_type<T>::value, "shader binding arighmetic type only support 32 bit arithmetic type");
		m_NumericDescriptors.push_back(std::make_pair(name, ShaderBindingDescriptor{
			is_shaderbinding_arighmetic_type<T>::numericType, 1, 2 }));
		return *this;
	}

	template<typename T>
	ShaderConstantsBuilder& Vec3(std::string const& name)
	{
		static_assert(is_shaderbinding_arighmetic_type<T>::value, "shader binding arighmetic type only support 32 bit arithmetic type");
		m_NumericDescriptors.push_back(std::make_pair(name, ShaderBindingDescriptor{
			is_shaderbinding_arighmetic_type<T>::numericType, 1, 3 }));
		return *this;
	}

	template<typename T, uint32_t count = 1>
	ShaderConstantsBuilder& Vec4(std::string const& name)
	{
		static_assert(is_shaderbinding_arighmetic_type<T>::value, "shader binding arighmetic type only support 32 bit arithmetic type");
		static_assert(count > 0, "shader binding count must be greater than 0");
		m_NumericDescriptors.push_back(std::make_pair(name, ShaderBindingDescriptor{ 
			is_shaderbinding_arighmetic_type<T>::numericType, count, 4 }));
		return *this;
	}

	template<typename T, uint32_t count = 1>
	ShaderConstantsBuilder& Mat4(std::string const& name)
	{
		static_assert(is_shaderbinding_arighmetic_type<T>::value, "shader binding arighmetic type only support 32 bit arithmetic type");
		static_assert(count > 0, "shader binding count must be greater than 0");
		m_NumericDescriptors.push_back(std::make_pair(name, ShaderBindingDescriptor{ 
			is_shaderbinding_arighmetic_type<T>::numericType, count, 4, 4 }));
		return *this;
	}

	std::string const& GetName() const { return m_Name; }

	bool operator==(ShaderConstantsBuilder const& rhs) const
	{
		return m_NumericDescriptors == rhs.m_NumericDescriptors
			&& m_Name == rhs.m_Name;
	}

	template <class HashAlgorithm>
	friend void hash_append(HashAlgorithm& h, ShaderConstantsBuilder const& bindingBuilder) noexcept
	{
		hash_append(h, bindingBuilder.m_Name);
		hash_append(h, bindingBuilder.m_NumericDescriptors);
	}

	std::vector<std::pair<std::string, ShaderBindingDescriptor>> const& GetNumericDescriptors() const{
		return m_NumericDescriptors;
	}
protected:
	std::string m_Name;
	std::vector<std::pair<std::string, ShaderBindingDescriptor>> m_NumericDescriptors;
};


class ShaderBindingBuilder
{
public:
	ShaderBindingBuilder(std::string const& space_name) : m_SpaceName(space_name) {}

	ShaderBindingBuilder& ConstantBuffer(ShaderConstantsBuilder const& constantDescs)
	{
		m_ConstantBufferDescriptors.emplace_back(constantDescs);
		return *this;
	}

	template<typename T, uint32_t channels = 4, uint32_t count = 1>
	ShaderBindingBuilder& Texture2D(std::string const& name)
	{
		static_assert(is_shaderbinding_arighmetic_type<T>::value, "shader binding arighmetic type only support 32 bit arithmetic type");
		static_assert(channels > 0 && channels <= 4, "texture binding supports 1-4 channels");
		static_assert(count > 0, "shader binding count must be greater than 0");
		m_TextureDescriptors.push_back(std::make_pair(name, ShaderTextureDescriptor{
		is_shaderbinding_arighmetic_type<T>::numericType
		, false, ETextureDimension::e2D, channels, count }));
		return *this;
	}

	template<typename T, uint32_t channels = 4, uint32_t count = 1>
	ShaderBindingBuilder& Texture3D(std::string const& name)
	{
		static_assert(is_shaderbinding_arighmetic_type<T>::value, "shader binding arighmetic type only support 32 bit arithmetic type");
		static_assert(channels > 0 && channels <= 4, "texture binding supports 1-4 channels");
		static_assert(count > 0, "shader binding count must be greater than 0");
		m_TextureDescriptors.push_back(std::make_pair(name, ShaderTextureDescriptor{
		is_shaderbinding_arighmetic_type<T>::numericType
		, false, ETextureDimension::e3D, channels, count }));
		return *this;
	}

	template<typename T, uint32_t channels = 4, uint32_t count = 1>
	ShaderBindingBuilder& RWTexture2D(std::string const& name)
	{
		static_assert(is_shaderbinding_arighmetic_type<T>::value, "shader binding arighmetic type only support 32 bit arithmetic type");
		static_assert(channels > 0 && channels <= 4, "texture binding supports 1-4 channels");
		static_assert(count > 0, "shader binding count must be greater than 0");
		m_TextureDescriptors.push_back(std::make_pair(name, ShaderTextureDescriptor{
		is_shaderbinding_arighmetic_type<T>::numericType
		, true, ETextureDimension::e2D, channels, count }));
		return *this;
	}

	template<typename T, uint32_t channels = 4, uint32_t count = 1>
	ShaderBindingBuilder& RWTexture3D(std::string const& name)
	{
		static_assert(is_shaderbinding_arighmetic_type<T>::value, "shader binding arighmetic type only support 32 bit arithmetic type");
		static_assert(channels > 0 && channels <= 4, "texture binding supports 1-4 channels");
		static_assert(count > 0, "shader binding count must be greater than 0");
		m_TextureDescriptors.push_back(std::make_pair(name, ShaderTextureDescriptor{
		is_shaderbinding_arighmetic_type<T>::numericType
		, true, ETextureDimension::e3D, channels, count }));
		return *this;
	}

	std::string const& GetSpaceName() const {
		return m_SpaceName;
	}

	std::vector<ShaderConstantsBuilder> const& GetConstantBufferDescriptors() const {
		return m_ConstantBufferDescriptors;
	}

	std::vector<std::pair<std::string, ShaderTextureDescriptor>> const& GetTextureDescriptors() const {
		return m_TextureDescriptors;
	}

	bool operator==(ShaderBindingBuilder const& rhs) const
	{
		return m_ConstantBufferDescriptors == rhs.m_ConstantBufferDescriptors
			&& m_TextureDescriptors == rhs.m_TextureDescriptors;
	}

	template <class HashAlgorithm>
	friend void hash_append(HashAlgorithm& h, ShaderBindingBuilder const& bindingBuilder) noexcept
	{
		hash_append(h, bindingBuilder.m_ConstantBufferDescriptors);
		hash_append(h, bindingBuilder.m_TextureDescriptors);
	}

protected:
	std::string m_SpaceName;
	std::vector<ShaderConstantsBuilder> m_ConstantBufferDescriptors;
	std::vector<std::pair<std::string, ShaderTextureDescriptor>> m_TextureDescriptors;
};

class ShaderBindingDescriptorList
{
public:
	ShaderBindingDescriptorList(std::initializer_list<ShaderBindingBuilder> binding_sets) : shaderBindings(binding_sets) {}
	
	std::vector<ShaderBindingBuilder> shaderBindings;
};