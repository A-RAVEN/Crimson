#pragma once
#include <include/Pipeline.h>
#include <headers/D3D12GPUDevice.h>

namespace Crimson
{
	class D3D12ShaderModule : public ShaderModule
	{
	public:
		friend class D3D12GPUDevice;
		friend class D3D12GraphicsPipeline;
		//friend class VulkanRenderPass;
		void Init(D3D12GPUDevice* p_device, ComPtr<ID3DBlob> shader_blob);
		void Init(D3D12GPUDevice* p_device, std::vector<char> const& shader_binary);
		virtual void Dispose() override;
	private:
		D3D12GPUDevice* m_Device;
		ComPtr<ID3DBlob> m_ShaderBlob;
		std::vector<char> m_Binary;
	};
}