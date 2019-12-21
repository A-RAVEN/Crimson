#include <GPUDevice.h>
#include <headers/Win32Window.h>

int main()
{
	using namespace Crimson;
	GPUDeviceManager::Init();
	GPUDeviceManager::Get()->InitAPIContext(EAPIType::E_API_TYPE_VULKAN, true);
	PGPUDevice MainDevice = GPUDeviceManager::Get()->CreateDevice("MainDevice", 0, EAPIType::E_API_TYPE_VULKAN, 3, 1, 1);
	Win32Window new_window;
	new_window.InitWindow(L"Test Window", L"default", 1024, 720);
	MainDevice->RegisterWindow(new_window);

	PGPUBuffer test_buffer = MainDevice->CreateBuffer(256, { EBufferUsage::E_BUFFER_USAGE_UNIFORM }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	test_buffer->Dispose();
	PGPUImage test_img0 = MainDevice->CreateImage(EFormat::E_FORMAT_R8G8B8A8_UNORM, 128, 128, 1, { EImageUsage::E_IMAGE_USAGE_COLOR_ATTACHMENT }, EMemoryType::E_MEMORY_TYPE_DEVICE);
	PRenderPass test_renderpass = MainDevice->CreateRenderPass();
	test_renderpass->m_Attachments = { {EFormat::E_FORMAT_B8G8R8A8_UNORM, EAttachmentClearType::E_ATTACHMENT_CLEAR_ZEROS}, {EFormat::E_FORMAT_D16_UNORM_S8_UINT, EAttachmentClearType::E_ATTACHMENT_CLEAR_ONES} };
	test_renderpass->m_Subpasses.resize(2);
	test_renderpass->m_Subpasses[0].m_OutputAttachments = { 0 };
	test_renderpass->m_Subpasses[0].m_DepthStencilAttachment = 1;

	test_renderpass->m_Subpasses[1].m_OutputAttachments = { 0 };
	test_renderpass->m_Subpasses[1].m_DepthStencilAttachment = 1;
	test_renderpass->m_Subpasses[1].m_DepthInputAttachments = { 1 };
	test_renderpass->m_Subpasses[1].m_StencilInputAttachments = { 1 };
	test_renderpass->BuildRenderPass();
	while (new_window.IsWindowRunning())
	{
		new_window.UpdateWindow();
	}
	return 0;
}