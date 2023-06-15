#include <cstdlib>
#include <windows.h>
#include <MRenderBackend.h>

typedef graphics_backend::CRenderBackend* (CALLBACK *FPT_NewBackend)();
typedef void(CALLBACK *FPT_DeleteBackend)(graphics_backend::CRenderBackend*);

HINSTANCE hRenderBackendLib = nullptr;
FPT_NewBackend pNewBackendFunc = nullptr;
FPT_DeleteBackend pDeleteBackendFunc = nullptr;

void LoadLibraries()
{
	hRenderBackendLib = LoadLibrary(L"VulkanRenderBackend");
	if(hRenderBackendLib != nullptr)
	{
		pNewBackendFunc = reinterpret_cast<FPT_NewBackend>(GetProcAddress(hRenderBackendLib, "NewBackend"));
		pDeleteBackendFunc = reinterpret_cast<FPT_DeleteBackend>(GetProcAddress(hRenderBackendLib, "DeleteBackend"));
	}
}

void UnloadLibraries()
{
	pNewBackendFunc = nullptr;
	pDeleteBackendFunc = nullptr;
	if (hRenderBackendLib != nullptr)
	{
		FreeLibrary(hRenderBackendLib);
		hRenderBackendLib = nullptr;
	}
}

int main(int argc, char *argv[])
{
	LoadLibraries();

	graphics_backend::CRenderBackend* pBackend = pNewBackendFunc();
	pBackend->Initialize("Test Vulkan Backend", "CRIMSON Engine");
	pBackend->Release();
	pDeleteBackendFunc(pBackend);

	UnloadLibraries();
	return EXIT_SUCCESS;
}