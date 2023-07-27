#include "private/include/pch.h"
#include "TestShaderProvider.h"

namespace graphics_backend
{
    uint64_t ShaderProvider_Impl::GetDataLength(std::string const& codeType) const
    {
        auto found = m_Data.find(codeType);
        if (found != m_Data.end())
        {
            return found->second.size();
        }
        return 0;
    }

    void const* ShaderProvider_Impl::GetDataPtr(std::string const& codeType) const
    {
        auto found = m_Data.find(codeType);
        if (found != m_Data.end())
        {
            return found->second.data();
        }
        return nullptr;
    }

    std::string ShaderProvider_Impl::GetUniqueName() const
    {
        return m_UniqueName;
    }
    void ShaderProvider_Impl::SetUniqueName(std::string const& uniqueName)
    {
        m_UniqueName = uniqueName;
    }
    void ShaderProvider_Impl::SetData(std::string const& codeType, void* dataPtr, uint64_t dataLength)
    {
        std::vector<char> data;
        data.resize(dataLength);
        memcpy(data.data(), dataPtr, dataLength);
        auto found = m_Data.find(codeType);
        if (found != m_Data.end())
        {
            found->second = data;
        }
		else
		{
			m_Data.insert(std::make_pair(codeType, data));
		}
    }
}