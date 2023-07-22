#pragma once
#include <memory>
#include <string>

#include "HashCommon.h"

namespace graphics_backend
{
	class ShaderProvider
	{
	public:
		virtual uint64_t GetDataLength(std::string const& codeType) const = 0;
		virtual void* GetDataPtr(std::string const& codeType) const = 0;
		virtual std::string GetSourcePath() const = 0;
	};

	class CShaderModule
	{
	public:
		std::shared_ptr<const ShaderProvider> GetShaderProvider() const
		{
			return m_Provider;
		}
		virtual void SetShaderProvider(std::shared_ptr<const ShaderProvider> provider)
		{
			m_Provider = provider;
		}
		virtual void Submit() = 0;
		virtual bool Done() = 0;

		bool operator==(CShaderModule const& other) const
		{
			return m_Provider == other.m_Provider;
		}
	protected:
		std::shared_ptr<const ShaderProvider> m_Provider = nullptr;
	};

	class CGraphicsShaderStates
	{
	public:
		std::shared_ptr<CShaderModule> vertexShader;
		std::shared_ptr<CShaderModule> fragmentShader;

		bool operator==(CGraphicsShaderStates const& other) const
		{
			return (vertexShader == other.vertexShader
				&& fragmentShader == other.fragmentShader);
		}
	};

	class CShaderStateLibrary
	{
		
	};
}

template<> struct std::hash<graphics_backend::ShaderProvider>
{
	std::size_t operator()(graphics_backend::ShaderProvider const& source) const noexcept
	{
		return std::hash<string>{}(source.GetSourcePath());
	}
};

template<> struct std::hash<graphics_backend::ShaderProvider*>
{
	std::size_t operator()(graphics_backend::ShaderProvider const* source) const noexcept
	{
		return source == nullptr ? 0 : std::hash<graphics_backend::ShaderProvider>{}(*source);
	}
};

template<> struct std::hash<graphics_backend::CShaderModule>
{
	std::size_t operator()(graphics_backend::CShaderModule const& source) const noexcept
	{
		return std::hash<graphics_backend::ShaderProvider*>{}(source.GetShaderProvider().get());
	}
};

template<> struct std::hash<graphics_backend::CShaderModule*>
{
	std::size_t operator()(graphics_backend::CShaderModule const* source) const noexcept
	{
		return source == nullptr ? 0 : std::hash<graphics_backend::CShaderModule>{}(*source);
	}
};

template<> struct std::hash<graphics_backend::CGraphicsShaderStates>
{
	std::size_t operator()(graphics_backend::CGraphicsShaderStates const& source) const noexcept
	{
		size_t h = std::hash<graphics_backend::CShaderModule*>{}(source.vertexShader.get());
		hash_combine(h, std::hash<graphics_backend::CShaderModule*>{}(source.fragmentShader.get()));
		return h;
	}
};