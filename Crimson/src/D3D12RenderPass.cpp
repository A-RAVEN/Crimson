#include <headers/D3D12RenderPass.h>
#include <headers/D3D12Pipeline.h>
#include <include/Generals.h>
#include <headers/D3D12Translator.h>
#include <headers/D3D12DebugLog.h>

namespace Crimson
{
	void D3D12RenderPass::BuildRenderPass()
	{
		std::vector<uint32_t> descriptorReferences;
		std::vector<bool> initially_referenced(m_Attachments.size(), false);
		std::vector<D3D12_RENDER_PASS_DEPTH_STENCIL_DESC> depthStencilDescriptors;
		std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> renderTargetDescriptors;
		//initial 
		for (uint32_t i = 0; i < m_Attachments.size(); ++i)
		{
			auto& attachement = m_Attachments[i];
			D3D12_RENDER_PASS_BEGINNING_ACCESS beginAccess{};
			D3D12_RENDER_PASS_ENDING_ACCESS endAccess{};
			if (IsDepthStencilFormat(attachement.m_Format))
			{
				switch (attachement.m_ClearType)
				{
				case EAttachmentClearType::E_ATTACHMENT_NOT_CLEAR:
					beginAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE::D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
					endAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
					break;
				case EAttachmentClearType::E_ATTACHMENT_CLEAR_ZEROS:
					beginAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE::D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
					beginAccess.Clear = { D3D12FormatType(attachement.m_Format) };
					beginAccess.Clear.ClearValue.DepthStencil = D3D12_DEPTH_STENCIL_VALUE{ 0.0f, 0x0 };
					endAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
					break;
				case EAttachmentClearType::E_ATTACHMENT_CLEAR_ONES:
					beginAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE::D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
					beginAccess.Clear = { D3D12FormatType(attachement.m_Format)};
					beginAccess.Clear.ClearValue.DepthStencil = D3D12_DEPTH_STENCIL_VALUE{ 1.0f, 0x0 };
					endAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
					break;
				}
				D3D12_RENDER_PASS_DEPTH_STENCIL_DESC newDesc{ D3D12_CPU_DESCRIPTOR_HANDLE{}, beginAccess, beginAccess, endAccess, endAccess };

				depthStencilDescriptors.push_back(newDesc);
				descriptorReferences.push_back(static_cast<uint32_t>(depthStencilDescriptors.size()) - 1);
			}
			else
			{
				switch (attachement.m_ClearType)
				{
				case EAttachmentClearType::E_ATTACHMENT_NOT_CLEAR:
					beginAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE::D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
					endAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
					break;
				case EAttachmentClearType::E_ATTACHMENT_CLEAR_ZEROS:
					beginAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE::D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
					beginAccess.Clear = { D3D12FormatType(attachement.m_Format), {0.0f, 0.0f, 0.0f, 0.0f} };
					endAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
					break;
				case EAttachmentClearType::E_ATTACHMENT_CLEAR_ONES:
					beginAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE::D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
					beginAccess.Clear = { D3D12FormatType(attachement.m_Format), {1.0f, 1.0f, 1.0f, 1.0f} };
					endAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
					break;
				}
				D3D12_RENDER_PASS_RENDER_TARGET_DESC newDesc{ D3D12_CPU_DESCRIPTOR_HANDLE{}, beginAccess, endAccess };
				renderTargetDescriptors.push_back(newDesc);
				descriptorReferences.push_back(static_cast<uint32_t>(renderTargetDescriptors.size()) - 1);
			}
		}
		m_D3D12Subpasses.resize(m_Subpasses.size());
		for (uint32_t subpassId = 0; subpassId < m_Subpasses.size(); ++subpassId)
		{
			auto& subpass = m_Subpasses[subpassId];
			auto& dxsubpass = m_D3D12Subpasses[subpassId];
			dxsubpass.m_RenderTargetDescriptors.clear();
			if (subpass.m_OutputAttachments.size() > 0)
			{
				D3D12_RT_FORMAT_ARRAY rtArray;
				rtArray.NumRenderTargets = subpass.m_OutputAttachments.size();
				for (int i = 0; i < subpass.m_OutputAttachments.size(); ++i)
				{
					auto& attachement = m_Attachments[subpass.m_OutputAttachments[i]];
					CRIM_ASSERT(IsColorFormat(attachement.m_Format), "D3D12 Error: Subpass" + std::to_string(subpassId) + " output attachment reference " + std::to_string(i) + " is not color format");
					auto& descriptor = renderTargetDescriptors[descriptorReferences[subpass.m_OutputAttachments[i]]];
					dxsubpass.m_RenderTargetDescriptors.push_back(descriptor);
					if (!initially_referenced[subpass.m_OutputAttachments[i]])
					{
						descriptor.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE::D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
						descriptor.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
						initially_referenced[subpass.m_OutputAttachments[i]] = true;
					}
					rtArray.RTFormats[i] = D3D12FormatType(attachement.m_Format);
				}
				CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rt_formats_stream = rtArray;
				dxsubpass.m_PipelineStateStreamingDataRenderTargetFormats.PushData(rt_formats_stream);
			}
			
			if (subpass.m_DepthStencilAttachment != -1)
			{

				auto& attachement = m_Attachments[subpass.m_DepthStencilAttachment];
				CRIM_ASSERT(IsDepthStencilFormat(attachement.m_Format), "D3D12 Error: Subpass" + std::to_string(subpassId) + " depth stencil reference " + " is not depth stencil format");
				auto& descriptor = depthStencilDescriptors[descriptorReferences[subpass.m_DepthStencilAttachment]];
				dxsubpass.m_DepthStencilDescriptor = descriptor;
				if (!initially_referenced[subpass.m_DepthStencilAttachment])
				{
					descriptor.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE::D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
					descriptor.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
					descriptor.StencilBeginningAccess = descriptor.DepthBeginningAccess;
					descriptor.StencilEndingAccess = descriptor.DepthEndingAccess;
					initially_referenced[subpass.m_DepthStencilAttachment] = true;
				}
				DXGI_FORMAT ds_format = D3D12FormatType(attachement.m_Format);
				CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT ds_format_stream = ds_format;
				dxsubpass.m_PipelineStateStreamingDataRenderTargetFormats.PushData(ds_format_stream);
			}
		}
	}
	void D3D12RenderPass::InstanciatePipeline(GraphicsPipeline* pipeline, uint32_t subpass)
	{
		auto& dxSubpass = m_D3D12Subpasses[subpass];
		auto find = dxSubpass.pipelineInstances.find(pipeline);
		if (find == dxSubpass.pipelineInstances.end())
		{
			D3D12GraphicsPipeline* dxpipeline = static_cast<D3D12GraphicsPipeline*>(pipeline);
			ByteVector stream = ByteVector::Combine(dxpipeline->m_PipelineStateStreamingData, dxSubpass.m_PipelineStateStreamingDataRenderTargetFormats);
			D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
				stream.Size(), stream.Data()
			};
			ComPtr<ID3D12PipelineState> pipelineState;
			CHECK_DXRESULT(p_OwningDevice->m_Device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&pipelineState)), "D3D12 Error: Pipeline Instantiation Issue On " + subpass);
			dxSubpass.pipelineInstances.insert(std::make_pair(pipeline, pipelineState));
		}
	}
	void D3D12RenderPass::Dispose()
	{
	}
	void D3D12RenderPass::InitRenderPass(D3D12GPUDevice* p_device)
	{
		p_OwningDevice = p_device;
	}
}