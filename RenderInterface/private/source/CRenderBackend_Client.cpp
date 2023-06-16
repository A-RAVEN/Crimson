#include  <private/include/pch.h>
#include "header/CRenderBackend.h"


namespace graphics_backend
{
	typedef CRenderBackend* (*FPT_NewBackend)();
	typedef void(*FPT_DeleteBackend)(CRenderBackend*);

	HINSTANCE hRenderBackendLib = nullptr;
	FPT_NewBackend pNewBackendFunc = nullptr;
	FPT_DeleteBackend pDeleteBackendFunc = nullptr;

	void LoadRenderBackend()
	{
		hRenderBackendLib = LoadLibrary(L"VulkanRenderBackend");
		if (hRenderBackendLib != nullptr)
		{
			pNewBackendFunc = reinterpret_cast<FPT_NewBackend>(GetProcAddress(hRenderBackendLib, "NewRenderBackend"));
			pDeleteBackendFunc = reinterpret_cast<FPT_DeleteBackend>(GetProcAddress(hRenderBackendLib, "DeleteRenderBackend"));
		}
	}

	void UnloadRenderBackend()
	{
		pNewBackendFunc = nullptr;
		pDeleteBackendFunc = nullptr;
		if (hRenderBackendLib != nullptr)
		{
			FreeLibrary(hRenderBackendLib);
			hRenderBackendLib = nullptr;
		}
	}

	CRenderBackend* NewRenderBackend()
	{
		return pNewBackendFunc();
	}

	void DeleteRenderBackend(CRenderBackend* removingBackend)
	{
		pDeleteBackendFunc(removingBackend);
	}
}

