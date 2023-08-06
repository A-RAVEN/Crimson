#include "TestShaderProvider.h"

uint64_t TestShaderProvider::GetDataLength(std::string const& codeType) const
{
    auto found = m_Data.find(codeType);
    if (found != m_Data.end())
    {
        return found->second.second.size();
    }
    return 0;
}

void const* TestShaderProvider::GetDataPtr(std::string const& codeType) const
{
    auto found = m_Data.find(codeType);
    if (found != m_Data.end())
    {
        return found->second.second.data();
    }
    return nullptr;
}

std::string TestShaderProvider::GetUniqueName() const
{
    return m_UniqueName;
}
ShaderProvider::ShaderSourceInfo const& TestShaderProvider::GetDataInfo(std::string const& codeType) const
{
    const static std::string invalidEntrypoint = "invalidEntryPoint";
    auto found = m_Data.find(codeType);
    if (found != m_Data.end())
    {
        return ShaderProvider::ShaderSourceInfo{
            found->second.second.size()
                , found->second.second.data()
                , found->second.first};
    }
    return ShaderProvider::ShaderSourceInfo{
        0
            , nullptr
            , invalidEntrypoint};
}
void TestShaderProvider::SetUniqueName(std::string const& uniqueName)
{
    m_UniqueName = uniqueName;
}
void TestShaderProvider::SetData(std::string const& codeType
    , std::string const& entryPoint
    , void* dataPtr
    , uint64_t dataLength)
{
    std::vector<char> data;
    data.resize(dataLength);
    memcpy(data.data(), dataPtr, dataLength);
    auto found = m_Data.find(codeType);
    if (found != m_Data.end())
    {
        found->second = std::make_pair(entryPoint, data);
    }
    else
    {
        m_Data.insert(std::make_pair(codeType, std::make_pair(entryPoint, data)));
    }
}