#pragma once
namespace graphics_backend
{
#pragma region Forward Declaration
	class CVulkanApplication;
	class CVulkanThreadContext;
	class CFrameCountContext;
#pragma endregion

	class ApplicationSubobjectBase
	{
	public:
		ApplicationSubobjectBase() = default;
		ApplicationSubobjectBase(ApplicationSubobjectBase const& other) = delete;
		ApplicationSubobjectBase& operator=(ApplicationSubobjectBase const&) = delete;
		ApplicationSubobjectBase(ApplicationSubobjectBase&& other) = default;
		ApplicationSubobjectBase& operator=(ApplicationSubobjectBase&&) = default;
		virtual void Initialize(CVulkanApplication const* owningApplication);
		virtual void Release();
		CVulkanApplication const* GetVulkanApplication() const;
		CFrameCountContext const& GetFrameCountContext() const;
		vk::Instance const& GetInstance() const;
		vk::Device const& GetDevice() const;
		vk::PhysicalDevice const& GetPhysicalDevice() const;
		CVulkanThreadContext* GetThreadContext(uint32_t threadIndex);
		virtual ~ApplicationSubobjectBase();
	protected:
		virtual void Initialize_Internal(CVulkanApplication const* owningApplication) = 0;
		virtual void Release_Internal() = 0;
		CVulkanApplication const* m_OwningApplication = nullptr;
	};

	struct ApplicationSubobjectBase_Deleter {
		void operator()(ApplicationSubobjectBase* deleteObject) {
			deleteObject->Release();
			delete deleteObject;
		}
	};
}